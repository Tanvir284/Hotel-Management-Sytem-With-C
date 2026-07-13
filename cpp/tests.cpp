// Non-interactive validation harness for the core domain logic.
// Not part of the app; compiled separately to verify feature parity.
#include "Booking.hpp"
#include "BookingRepository.hpp"
#include "DateUtil.hpp"
#include "RoomCatalog.hpp"

#include <cassert>
#include <cstdio>
#include <string>

static int failures = 0;
#define CHECK(cond, msg)                                                        \
    do {                                                                        \
        if (!(cond)) { std::printf("FAIL: %s\n", msg); ++failures; }           \
        else { std::printf("ok  : %s\n", msg); }                               \
    } while (0)

int main() {
    // --- RoomCatalog: ranges & pricing match the original C code ---------
    CHECK(RoomCatalog::typeOf(1)->pricePerNight() == 3000, "room 1 = 3000 (King/Queen)");
    CHECK(RoomCatalog::typeOf(20)->pricePerNight() == 3000, "room 20 = 3000");
    CHECK(RoomCatalog::typeOf(21)->pricePerNight() == 2000, "room 21 = 2000 (Deluxe)");
    CHECK(RoomCatalog::typeOf(35)->pricePerNight() == 2000, "room 35 = 2000");
    CHECK(RoomCatalog::typeOf(36)->pricePerNight() == 2700, "room 36 = 2700 (Triple)");
    CHECK(RoomCatalog::typeOf(55)->pricePerNight() == 2700, "room 55 = 2700");
    CHECK(RoomCatalog::typeOf(56)->pricePerNight() == 3600, "room 56 = 3600 (Quad)");
    CHECK(RoomCatalog::typeOf(75)->pricePerNight() == 3600, "room 75 = 3600");
    CHECK(RoomCatalog::typeOf(76)->pricePerNight() == 1000, "room 76 = 1000 (Single)");
    CHECK(RoomCatalog::typeOf(90)->pricePerNight() == 1000, "room 90 = 1000");
    CHECK(RoomCatalog::typeOf(91)->pricePerNight() == 1800, "room 91 = 1800 (Twin)");
    CHECK(RoomCatalog::typeOf(100)->pricePerNight() == 1800, "room 100 = 1800");
    CHECK(RoomCatalog::typeOf(1)->isAC(), "room 1 is AC");
    CHECK(!RoomCatalog::typeOf(80)->isAC(), "room 80 is Non-AC");

    // --- Booking pricing: mirrors Calc_Amount() --------------------------
    {
        Booking b;
        b.setRoomNo(5);            // King/Queen, 3000/night
        b.setPeriod(4);            // 4 nights
        b.setLaundry(2);           // laundry every 2 days
        b.setGym(3);               // 3 gym days
        b.recomputeRoomPrice();
        // room = 4*3000 = 12000; laundry = ceil(4/2)*500 = 1000; gym = 3*1000=3000
        CHECK(b.roomPrice() == 12000, "roomPrice 4 nights King = 12000");
        CHECK(b.laundryCost() == 1000, "laundryCost period4 freq2 = 1000");
        CHECK(b.gymCost() == 3000, "gymCost 3 days = 3000");
        CHECK(b.total() == 16000, "total = room+laundry+gym = 16000");
        b.setPayment(6000);
        CHECK(b.balanceDue() == 10000, "balanceDue after 6000 advance = 10000");
        b.addFood(750);
        CHECK(b.total() == 16750, "total after 750 food = 16750");
    }

    // --- Division-by-zero guard: laundry 0 must not crash ----------------
    {
        Booking b;
        b.setRoomNo(1);
        b.setPeriod(3);
        b.setLaundry(0);
        b.recomputeRoomPrice();
        CHECK(b.laundryCost() == 0, "laundryCost with freq 0 is 0 (no crash)");
    }

    // --- Serialization round-trip (text persistence) ---------------------
    {
        Booking b;
        b.setRoomNo(42);
        b.setPeriod(5);
        b.setLaundry(3);
        b.setGym(2);
        b.addGuest(Guest("Alice O'Neil", "Female", 30));
        b.addGuest(Guest("Bob | Pipe\\Slash", "Male", 25)); // tricky chars
        b.setAddress("12 Main St | Apt 3");
        b.setPhone("017-1234567");
        b.setCheckIn(10, 7, 2026, 14, 30);
        b.setPayment(1000);
        b.recomputeRoomPrice();

        std::string line = b.serialize();
        Booking c = Booking::deserialize(line);
        CHECK(c.roomNo() == 42, "round-trip roomNo");
        CHECK(c.capacity() == 2, "round-trip guest count");
        CHECK(c.guests()[0].name() == "Alice O'Neil", "round-trip guest0 name");
        CHECK(c.guests()[1].name() == "Bob | Pipe\\Slash", "round-trip escaped name");
        CHECK(c.address() == "12 Main St | Apt 3", "round-trip escaped address");
        CHECK(c.phone() == "017-1234567", "round-trip phone");
        CHECK(c.period() == 5, "round-trip period");
        CHECK(c.total() == b.total(), "round-trip total matches");
    }

    // --- DateUtil check-out math (mirrors Put_date) ----------------------
    {
        Date out = DateUtil::addDays({28, 2, 2024}, 2); // leap year Feb
        CHECK(out.day == 1 && out.month == 3 && out.year == 2024,
              "28 Feb 2024 + 2 days = 1 Mar 2024 (leap)");
        Date out2 = DateUtil::addDays({28, 2, 2023}, 2); // non-leap
        CHECK(out2.day == 2 && out2.month == 3 && out2.year == 2023,
              "28 Feb 2023 + 2 days = 2 Mar 2023 (non-leap)");
        Date out3 = DateUtil::addDays({31, 12, 2025}, 1);
        CHECK(out3.day == 1 && out3.month == 1 && out3.year == 2026,
              "31 Dec 2025 + 1 day = 1 Jan 2026 (year roll)");
    }

    // --- Repository: append / find / saveAll / occupancy -----------------
    {
        const char* testFile = "test_records.dat";
        std::remove(testFile);
        BookingRepository repo(testFile);

        Booking b1; b1.setRoomNo(10); b1.setPeriod(2); b1.setLaundry(1);
        b1.addGuest(Guest("Carol", "Female", 40)); b1.recomputeRoomPrice();
        repo.append(b1);

        Booking b2; b2.setRoomNo(80); b2.setPeriod(1); b2.setLaundry(1);
        b2.addGuest(Guest("Dan", "Male", 22)); b2.recomputeRoomPrice();
        repo.append(b2);

        CHECK(repo.loadAll().size() == 2, "repo has 2 records");
        CHECK(repo.isOccupied(10), "room 10 occupied");
        CHECK(!repo.isOccupied(99), "room 99 free");

        Booking found;
        CHECK(repo.findByRoom(80, found) && found.guests()[0].name() == "Dan",
              "findByRoom returns correct record");

        // Remove room 10 by saving filtered list (checkout behaviour)
        auto all = repo.loadAll();
        all.erase(all.begin());
        repo.saveAll(all);
        CHECK(repo.loadAll().size() == 1, "after checkout, 1 record left");
        CHECK(!repo.isOccupied(10), "room 10 freed after checkout");

        std::remove(testFile);
    }

    // --- Coupon discount applies to full bill, not just room price ---------
    {
        Booking b;
        b.setRoomNo(76);
        b.setPeriod(1);
        b.recomputeRoomPrice();
        b.setFoodPrice(9000);
        int totalBefore = b.total();
        int discount = (totalBefore * 20) / 100;
        b.addDiscount(discount);
        CHECK(b.total() == totalBefore - discount,
              "coupon discount reduces full total");
    }

    std::printf("\n%s (failures=%d)\n", failures == 0 ? "ALL TESTS PASSED" : "SOME TESTS FAILED", failures);
    return failures == 0 ? 0 : 1;
}
