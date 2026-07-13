#include "HotelService.hpp"
#include "FoodMenu.hpp"
#include "RoomCatalog.hpp"

#include <algorithm>
#include <cctype>
#include <fstream>

namespace {
const char* kConfigFile = "admin.cfg";
const char* kRecordFile = "Record.dat";
const char* kHistoryFile = "History.dat";

std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
} // namespace

HotelService::HotelService()
    : auth_(kConfigFile), records_(kRecordFile), history_(kHistoryFile) {}

std::vector<Booking> HotelService::activeBookings() const {
    return records_.loadAll();
}

std::vector<Booking> HotelService::checkoutHistory() const {
    return history_.loadAll();
}

std::vector<int> HotelService::bookedRooms() const {
    std::vector<int> rooms;
    for (const auto& b : records_.loadAll()) rooms.push_back(b.roomNo());
    std::sort(rooms.begin(), rooms.end());
    return rooms;
}

std::vector<int> HotelService::emptyRooms() const {
    std::vector<int> booked = bookedRooms();
    std::vector<int> empty;
    for (int r = RoomCatalog::kMinRoom; r <= RoomCatalog::kMaxRoom; ++r) {
        if (!std::binary_search(booked.begin(), booked.end(), r))
            empty.push_back(r);
    }
    return empty;
}

bool HotelService::isOccupied(int room) const {
    return records_.isOccupied(room);
}

bool HotelService::findBooking(int room, Booking& out) const {
    return records_.findByRoom(room, out);
}

void HotelService::saveBooking(const Booking& booking) {
    if (records_.isOccupied(booking.roomNo())) {
        std::vector<Booking> all = records_.loadAll();
        for (auto& b : all) {
            if (b.roomNo() == booking.roomNo()) b = booking;
        }
        records_.saveAll(all);
    } else {
        records_.append(booking);
    }
}

bool HotelService::checkout(int room, Booking& checkedOut) {
    std::vector<Booking> all = records_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
                           [room](const Booking& b) { return b.roomNo() == room; });
    if (it == all.end()) return false;
    checkedOut = *it;
    writeReceipt(checkedOut);
    history_.append(checkedOut);
    all.erase(it);
    records_.saveAll(all);
    return true;
}

std::vector<Booking> HotelService::searchGuests(const std::string& query) const {
    std::vector<Booking> matches;
    const std::string q = toLower(query);
    if (q.empty()) return matches;
    for (const auto& b : records_.loadAll()) {
        for (const auto& g : b.guests()) {
            if (toLower(g.name()).find(q) != std::string::npos) {
                matches.push_back(b);
                break;
            }
        }
    }
    return matches;
}

bool HotelService::applyCoupon(int room, const std::string& code, int& discountOut) {
    int percent = 0;
    if (code == "WELCOME10") percent = 10;
    else if (code == "STAY20") percent = 20;
    else return false;

    Booking b;
    if (!records_.findByRoom(room, b)) return false;

    discountOut = (b.total() * percent) / 100;
    b.addDiscount(discountOut);

    std::vector<Booking> all = records_.loadAll();
    for (auto& item : all) {
        if (item.roomNo() == room) item = b;
    }
    records_.saveAll(all);
    return true;
}

bool HotelService::assignRooms(int totalGuests, bool wantAC, int period,
                               std::vector<int>& roomsOut) const {
    (void)period;
    roomsOut.clear();
    std::vector<int> empty = emptyRooms();
    std::vector<int> candidates;
    for (int r : empty) {
        bool ac = RoomCatalog::typeOf(r)->isAC();
        if (wantAC == ac) candidates.push_back(r);
    }
    std::sort(candidates.begin(), candidates.end());

    int remaining = totalGuests;
    for (int r : candidates) {
        if (remaining <= 0) break;
        roomsOut.push_back(r);
        remaining -= RoomCatalog::typeOf(r)->capacity();
    }
    return remaining <= 0;
}

std::string HotelService::roomSummary(int room) {
    if (!RoomCatalog::isValid(room)) return "Invalid room";
    auto type = RoomCatalog::typeOf(room);
    return "Room " + std::to_string(room) + " - " + type->name() + " (" +
           (type->isAC() ? "AC" : "Non-AC") + ", " +
           std::to_string(type->pricePerNight()) + "/night)";
}

bool HotelService::addFoodOrder(int room, int itemIndex1Based, int qty, int& amountOut) {
    const auto& menu = foodMenu();
    if (itemIndex1Based < 1 || itemIndex1Based > static_cast<int>(menu.size()) || qty < 1)
        return false;
    Booking b;
    if (!records_.findByRoom(room, b)) return false;
    amountOut = menu[static_cast<size_t>(itemIndex1Based - 1)].price * qty;
    b.addFood(amountOut);
    saveBooking(b);
    return true;
}

bool HotelService::modifyGuest(int room, int guestIndex1Based, const std::string& name,
                               const std::string& gender, int age,
                               const std::string& address, const std::string& phone,
                               int periodDays) {
    Booking b;
    if (!records_.findByRoom(room, b)) return false;
    if (guestIndex1Based < 1 || guestIndex1Based > b.capacity()) return false;
    Guest& g = b.guests()[guestIndex1Based - 1];
    if (!name.empty()) g.setName(name);
    if (!gender.empty()) g.setGender(gender);
    if (age > 0) g.setAge(age);
    if (!address.empty()) b.setAddress(address);
    if (!phone.empty()) b.setPhone(phone);
    if (periodDays > 0 && periodDays <= 60) {
        b.setPeriod(periodDays);
        if (b.gym() > periodDays) b.setGym(periodDays);
        b.recomputeRoomPrice();
    }
    saveBooking(b);
    return true;
}

bool HotelService::setLaundryFrequency(int room, int frequency) {
    if (frequency < 1 || frequency > 4) return false;
    Booking b;
    if (!records_.findByRoom(room, b)) return false;
    b.setLaundry(frequency);
    b.recomputeRoomPrice();
    saveBooking(b);
    return true;
}

bool HotelService::upgradeRoom(int fromRoom, int toRoom) {
    if (!RoomCatalog::isValid(toRoom) || records_.isOccupied(toRoom)) return false;
    Booking b;
    if (!records_.findByRoom(fromRoom, b)) return false;
    std::vector<Booking> all = records_.loadAll();
    for (auto& rec : all) {
        if (rec.roomNo() == fromRoom) {
            rec.setRoomNo(toRoom);
            rec.recomputeRoomPrice();
            records_.saveAll(all);
            return true;
        }
    }
    return false;
}

std::string HotelService::roomFeaturesText(int room) {
    if (!RoomCatalog::isValid(room)) return "Invalid room number.";
    auto type = RoomCatalog::typeOf(room);
    std::string out = "Room Type: " + type->name() + "\n";
    out += "Beds: " + std::to_string(type->capacity()) + "\n";
    out += "Capacity: " + std::to_string(type->capacity()) + " guest(s)\n";
    out += type->isAC() ? "A.C. Type\n" : "Non A.C. Type\n";
    out += "Price per night: " + std::to_string(type->pricePerNight()) +
           "/- (Including Breakfast)\n";
    for (const auto& f : type->features()) out += " -> " + f + "\n";
    return out;
}

std::string HotelService::writeReceipt(const Booking& b) {
    std::string path = "Receipt_Room" + std::to_string(b.roomNo()) + ".txt";
    std::ofstream rc(path);
    if (!rc) return "";
    rc << "========================================\n";
    rc << "       THE RUHIT'S PALACE - RECEIPT\n";
    rc << "========================================\n\n";
    rc << " Room No: " << b.roomNo() << "\n";
    rc << " Guest Name: " << (b.capacity() > 0 ? b.guests()[0].name() : "") << "\n";
    rc << " Check-in: " << b.checkInDay() << "/" << b.checkInMonth() << "/"
       << b.checkInYear() << " at " << b.checkInHour() << ":" << b.checkInMinute() << "\n";
    rc << " Duration: " << b.period() << " days\n\n";
    rc << " Room Charges:    " << b.roomPrice() << "\n";
    rc << " Food Charges:    " << b.foodPrice() << "\n";
    rc << " Laundry:         " << b.laundryCost() << "\n";
    rc << " Gym:             " << b.gymCost() << "\n";
    if (b.discount() > 0) rc << " Discount:        " << b.discount() << "\n";
    rc << "----------------------------------------\n";
    rc << " TOTAL:           " << b.total() << "\n";
    rc << " Advance Paid:    " << b.payment() << "\n";
    rc << " BALANCE DUE:     " << b.balanceDue() << "\n";
    rc << "========================================\n";
    rc << "         Thank you for your stay!\n";
    return path;
}
