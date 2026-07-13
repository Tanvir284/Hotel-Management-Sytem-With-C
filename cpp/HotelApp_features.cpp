#include "HotelApp.hpp"
#include "DateUtil.hpp"
#include "RoomCatalog.hpp"
#include "Terminal.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace {
char readChar2() {
    std::string line = Terminal::readLine();
    for (char c : line)
        if (!std::isspace(static_cast<unsigned char>(c))) return c;
    return '\0';
}
} // namespace

// --- Collect guest details (was Getdata) -----------------------------------
void HotelApp::collectGuests(Booking& b, int count, bool primary) {
    Terminal::clear();
    std::cout << "\n Booking " << count << " Members in Room No. " << b.roomNo()
              << "\n";

    for (int i = 0; i < count; ++i) {
        std::cout << "\n Enter details of Guest " << (i + 1) << "\n";
        std::cout << " Name : ";
        std::string name = Terminal::readLine();

        std::string gender;
        while (gender.empty()) {
            std::cout << " Gender (M/F/T): ";
            char g = readChar2();
            if (g == 'M' || g == 'm') gender = "Male";
            else if (g == 'F' || g == 'f') gender = "Female";
            else if (g == 'T' || g == 't') gender = "Transgender";
            else std::cout << " Incorrect option\n";
        }
        int age = Terminal::readInt(" Age: ");
        b.addGuest(Guest(name, gender, age));

        if (i == 0) {
            int laundry = 0;
            do {
                laundry = Terminal::readInt(
                    "\n Enter Laundry Frequency (1-4 days): ");
                if (laundry < 1 || laundry > 4)
                    std::cout << " Invalid laundry frequency. Enter 1 to 4.\n";
            } while (laundry < 1 || laundry > 4);
            b.setLaundry(laundry);

            std::cout << "\n Do you want Fitness Center (Gym) access (Y/N): ";
            char gy = readChar2();
            if (gy == 'Y' || gy == 'y') {
                int gymDays = Terminal::readInt(
                    "\n Enter No. of days of gym access (stay is " +
                    std::to_string(b.period()) + "): ");
                if (gymDays < 0) gymDays = 0;
                if (gymDays > b.period()) gymDays = b.period();
                b.setGym(gymDays);
            } else {
                b.setGym(0);
            }

            b.recomputeRoomPrice();

            while (true) {
                std::cout << "\n Total Amount Due is " << b.total();
                std::cout << "\n Do Guest 1 want to Pay in Advance (Y/N): ";
                char pay = readChar2();
                if (pay == 'Y' || pay == 'y') {
                    int advance = Terminal::readInt("\n Enter Advance Payment: ");
                    if (advance < 0) advance = 0;
                    if (advance > b.total()) advance = b.total();
                    b.setPayment(advance);
                    break;
                } else if (pay == 'N' || pay == 'n') {
                    b.setPayment(0);
                    std::cout << "\n Total Payable Amount is: " << b.balanceDue()
                              << "\n";
                    break;
                } else {
                    std::cout << "\n Wrong Choice! Please Enter correct option.";
                }
            }
        }

        if (i == 0 && primary) {
            std::cout << "\n Address: ";
            b.setAddress(Terminal::readLine());
            std::cout << " Phone No: ";
            b.setPhone(Terminal::readLine());
        }
    }

    Date d = DateUtil::now();
    auto t = DateUtil::nowTime();
    b.setCheckIn(d.day, d.month, d.year, t[0], t[1]);
}

// --- Book a room (was Add + Room_opt) --------------------------------------
void HotelApp::addBooking() {
    Terminal::setColor("5E");
    Terminal::clear();
    std::cout << "\n NOTE: Maximum No. of Guests at a time is 10\n\n";

    int adults = Terminal::readInt(" Enter No. of Adult guests: ");
    if (adults < 0) adults = 0;
    int children = 0;
    if (adults <= 10) {
        children = Terminal::readInt(" Enter No. of children: ");
        if (children < 0) children = 0;
        std::cout << " NOTE: Free entry for children under 11 years.\n\n";
    }

    int underAge = 0;
    for (int i = 0; i < children; ++i) {
        int age = Terminal::readInt(" Enter age of child " +
                                    std::to_string(i + 1) + ": ");
        if (age <= 11) ++underAge;
    }

    int total = adults + children;
    if (underAge > 0) {
        std::cout << "\n Extra bed for " << underAge
                  << " children below 11 years (Y/N): ";
        char ans = readChar2();
        if (!(ans == 'Y' || ans == 'y')) total -= underAge;
    }

    if (total < 1) {
        Terminal::pause("\n No guests to book. Press any key.");
        return;
    }
    if (total > 10) {
        Terminal::pause("\n Total guests exceed 10. Booking cancelled. Press any key.");
        return;
    }

    int period = 0;
    do {
        period = Terminal::readInt("\n Duration(days) of Stay (Max. 60): ");
        if (period < 1 || period > 60)
            std::cout << " Stay must be between 1 and 60 days. Enter Again!\n";
    } while (period < 1 || period > 60);

    std::cout << "\n Enter Guest Room type choice:\n";
    int choice = Terminal::readInt(" 1.A.C. \n 2.Non- A.C.\n -> ");

    // Determine candidate free rooms for the chosen AC/Non-AC category.
    std::vector<int> empty = emptyRooms();
    std::vector<int> candidates;
    for (int r : empty) {
        bool ac = RoomCatalog::typeOf(r)->isAC();
        if ((choice == 1 && ac) || (choice == 2 && !ac)) candidates.push_back(r);
    }

    // Greedily assign the smallest-capacity rooms that together seat everyone.
    // Rooms are considered in ascending number order (matching original ranges).
    std::sort(candidates.begin(), candidates.end());
    int remaining = total;
    std::vector<int> assigned;
    for (int r : candidates) {
        if (remaining <= 0) break;
        assigned.push_back(r);
        remaining -= RoomCatalog::typeOf(r)->capacity();
    }

    if (remaining > 0) {
        Terminal::pause("\n Not enough free rooms for this many guests. Press any key.");
        return;
    }

    // Fill the assigned rooms with guests.
    int guestsLeft = total;
    bool primary = true;
    for (int r : assigned) {
        int cap = RoomCatalog::typeOf(r)->capacity();
        int here = std::min(cap, guestsLeft);
        Booking b;
        b.setRoomNo(r);
        b.setPeriod(period);
        collectGuests(b, here, primary);
        b.recomputeRoomPrice();
        records_.append(b);
        guestsLeft -= here;
        primary = false;
        std::cout << "\n\n Room " << r << " successfully booked!!!";
    }
    Terminal::pause();
}

// --- Display a record (was Display) ----------------------------------------
void HotelApp::displayRecord() {
    Terminal::clear();
    if (records_.loadAll().empty()) {
        Terminal::pause("\n No Records Found!! All Rooms are Empty.");
        return;
    }
    int r = promptRoomNumber();
    if (r == -1) return;

    Booking b;
    if (records_.findByRoom(r, b)) {
        printBooking(b);
    } else {
        std::cout << "\n Sorry the Room is vacant.";
    }
    Terminal::pause();
}

// --- Room status (was Room_stat / Booking_stat) ----------------------------
void HotelApp::roomStatus() {
    std::vector<int> booked = bookedRooms();
    std::vector<int> empty = emptyRooms();

    Terminal::clear();
    std::cout << "\n\t\t\t Rooms Status";
    std::cout << "\n\t\t\t**************\n\n";
    std::cout << "\t Total no of booked rooms are : " << booked.size() << "\n";
    std::cout << "\t Total no of empty rooms are  : " << empty.size();

    std::cout << "\n\n\tEnter your choice to display Room Status";
    std::cout << "\n\t1. Display Booked Rooms";
    std::cout << "\n\t2. Display Empty Rooms\n\t->";
    int ch = Terminal::readInt("");

    const std::vector<int>& list = (ch == 1) ? booked : empty;
    if (ch != 1 && ch != 2) {
        Terminal::pause("\n Wrong Choice.");
        return;
    }
    std::cout << (ch == 1 ? "\n\n List of Rooms booked:\n" : "\n\n List of Empty rooms:\n");
    int col = 0;
    for (int r : list) {
        std::cout << "  " << r << " (" << RoomCatalog::typeOf(r)->name() << ")\t";
        if (++col % 3 == 0) std::cout << "\n";
    }
    Terminal::pause();
}

// --- Room features (was Room_Features) -------------------------------------
void HotelApp::roomFeatures() {
    Terminal::clear();
    int r = 0;
    while (!RoomCatalog::isValid(r)) {
        r = Terminal::readInt("\n Enter a Room No. (1-100): ");
        if (!RoomCatalog::isValid(r))
            std::cout << "\n Wrong Input! Enter a value between 1 and 100.";
    }
    auto type = RoomCatalog::typeOf(r);
    std::cout << "\n\n Room  Type: " << type->name();
    std::cout << "\n Number of Beds: " << type->capacity();
    std::cout << "\n Room Capacity: " << type->capacity() << " Guest";
    std::cout << "\n A.C./Non A.C.: " << (type->isAC() ? "A.C. Type" : "Non A.C. Type");
    std::cout << "\n Price per night: " << type->pricePerNight()
              << "/- (Including Breakfast)";
    auto feats = type->features();
    if (!feats.empty()) {
        std::cout << "\n\n Additional Features :";
        for (const auto& f : feats) std::cout << "\n -> " << f;
    }
    Terminal::pause();
}
