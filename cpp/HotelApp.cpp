#include "HotelApp.hpp"
#include "DateUtil.hpp"
#include "RoomCatalog.hpp"
#include "Terminal.hpp"

#include <algorithm>
#include <iostream>
#include <string>

namespace {
const char* kConfigFile = "admin.cfg";
const char* kRecordFile = "Record.dat";
const char* kHistoryFile = "History.dat";
} // namespace

HotelApp::HotelApp()
    : auth_(kConfigFile), records_(kRecordFile), history_(kHistoryFile) {}

int HotelApp::run() {
    welcomeScreen();
    Terminal::clear();
    if (auth_.login()) {
        mainMenu();
    } else {
        Terminal::clear();
        std::cout << "\n\n\t\t\t\t\t    THANK YOU\n";
    }
    return 0;
}

void HotelApp::welcomeScreen() {
    Terminal::setColor("A1");
    std::string load = "LOADING....";
    std::cout << "\n\n\n\n\n\n\t\t\t\t\t\t    ";
    for (char c : load) {
        Terminal::delayMs(60);
        std::cout << c << std::flush;
    }
    Terminal::delayMs(50);
    Terminal::clear();

    std::cout << "\n\t\t\t\t\t";
    for (int i = 0; i < 34; ++i) std::cout << "*";
    std::cout << "\n\t\t\t\t\t   * HOTEL MANAGEMENT PROJECT *\n";
    std::cout << "\t\t\t\t\t";
    for (int i = 0; i < 34; ++i) std::cout << "*";
    std::cout << "\n";
    for (int i = 0; i < 120; ++i) std::cout << "_";
    std::cout << "\n\n\n\t\t\t\t\t Made by- \n\t\t\t\t\t       1.MD Tanvir Islam\n";
    for (int i = 0; i < 120; ++i) std::cout << "_";
    Terminal::pause("\n\n\t\t\t Press any key to continue...");
}

void HotelApp::mainMenu() {
    int choice = 0;
    while (choice != 10) {
        Terminal::setColor("E1");
        Terminal::clear();
        std::cout << "\n\n\t\t\t\t\t       ************************\n";
        std::cout << "\t\t\t\t\t        *THE RUHIT'S PALACE *";
        std::cout << "\n\t\t\t\t\t       ************************\n\n\n";
        std::cout << "\n\t\t\t\t\t\t *************";
        std::cout << "\n\t\t\t\t\t\t * MAIN MENU *";
        std::cout << "\n\t\t\t\t\t\t *************\n\n";
        std::cout << "\n\t\t\t\t\t\t1. Book A Room\n";
        std::cout << "\n\t\t\t\t\t\t2. Display Room Record\n";
        std::cout << "\n\t\t\t\t\t\t3. Rooms Status\n";
        std::cout << "\n\t\t\t\t\t\t4. Room Features\n";
        std::cout << "\n\t\t\t\t\t\t5. Modify Room Record\n";
        std::cout << "\n\t\t\t\t\t\t6. Check-Out\n";
        std::cout << "\n\t\t\t\t\t\t7. Search Guest by Name\n";
        std::cout << "\n\t\t\t\t\t\t8. Guest History\n";
        std::cout << "\n\t\t\t\t\t\t9. Apply Discount Coupon\n";
        std::cout << "\n\t\t\t\t\t\t10. Exit";
        choice = Terminal::readInt("\n\n\n\t\t\t\t\t\tEnter Your Choice: ");

        try {
            switch (choice) {
                case 1: addBooking(); break;
                case 2: displayRecord(); break;
                case 3: roomStatus(); break;
                case 4: roomFeatures(); break;
                case 5: modifyMenu(); break;
                case 6: checkOut(); break;
                case 7: searchGuest(); break;
                case 8: viewHistory(); break;
                case 9: applyCouponFlow(); break;
                case 10:
                    Terminal::clear();
                    std::cout << "\n\n\n\t\t\t\t\t    THANK YOU\n";
                    std::cout << "\t\t\t\t\t   Visit Again.\n\n";
                    break;
                default:
                    Terminal::pause("\n\n\t\t\tWrong choice!!! Press any key.");
            }
        } catch (const std::exception& e) {
            std::cout << "\n Error: " << e.what() << "\n";
            Terminal::pause();
        }
    }
}

// --- Helpers ---------------------------------------------------------------

std::vector<int> HotelApp::bookedRooms() const {
    std::vector<int> rooms;
    for (const auto& b : records_.loadAll()) rooms.push_back(b.roomNo());
    std::sort(rooms.begin(), rooms.end());
    return rooms;
}

std::vector<int> HotelApp::emptyRooms() const {
    std::vector<int> booked = bookedRooms();
    std::vector<int> empty;
    for (int r = RoomCatalog::kMinRoom; r <= RoomCatalog::kMaxRoom; ++r) {
        if (!std::binary_search(booked.begin(), booked.end(), r))
            empty.push_back(r);
    }
    return empty;
}

int HotelApp::promptRoomNumber() {
    while (true) {
        int r = Terminal::readInt("\n Enter room no: ");
        if (RoomCatalog::isValid(r)) return r;
        std::cout << "\n The entered room no does not exist.";
        std::cout << "\n Please enter a valid room no\n\t or\n Press esc to exit\n";
        int ch = Terminal::getKey();
        if (ch == 27) return -1;
    }
}

void HotelApp::printBooking(const Booking& b) const {
    Terminal::clear();
    std::cout << "\n  Customer Details";
    std::cout << "\n  ****************\n";
    std::cout << "\n Room no: " << b.roomNo();
    std::cout << "\n No. Guests in Room Booked: " << b.capacity();

    for (int i = 0; i < b.capacity(); ++i) {
        const Guest& g = b.guests()[i];
        std::cout << "\n\n Details of Guest " << (i + 1);
        std::cout << "\n  Name: " << g.name();
        std::cout << "\n  Gender: " << g.gender();
        std::cout << "\n  Age:  " << g.age();
        if (!b.address().empty()) {
            std::cout << "\n  Address: " << b.address();
            std::cout << "\n  Phone no: " << b.phone();
        }
    }
    std::cout << "\n\n Duration(days): " << b.period();
    std::cout << "\n Laundry frequency: " << b.laundry();
    std::cout << "\n No. of days of Gym accessibility: " << b.gym();
    std::cout << "\n Food Amount Payable = " << b.foodPrice();
    std::cout << "\n\n Total Expense of Room is: " << b.total();
    std::cout << "\n Total Amount Payable = " << b.balanceDue();

    std::cout << "\n\n Check in date is : " << b.checkInDay() << "/"
              << b.checkInMonth() << "/" << b.checkInYear();
    std::cout << "\n Check in Time is : " << b.checkInHour() << ":"
              << b.checkInMinute();

    Date out = DateUtil::addDays(
        {b.checkInDay(), b.checkInMonth(), b.checkInYear()}, b.period());
    std::cout << "\n Check out date is : " << out.day << "/" << out.month << "/"
              << out.year;
}
