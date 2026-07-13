#ifndef HOTELAPP_HPP
#define HOTELAPP_HPP

#include "Auth.hpp"
#include "Booking.hpp"
#include "BookingRepository.hpp"

#include <vector>

// Top-level application controller. Owns the persistence repositories and the
// auth service, and implements every menu feature from the original C program:
// booking, display, room status, features, modify (food/data/laundry/upgrade),
// checkout, search, history and coupons.
class HotelApp {
public:
    HotelApp();

    // Runs the full program: welcome screen, login, main menu loop.
    int run();

private:
    // Menu features -------------------------------------------------------
    void welcomeScreen();
    void mainMenu();
    void addBooking();          // Add()
    void displayRecord();       // Display()
    void roomStatus();          // Room_stat(1)
    void roomFeatures();        // Room_Features()
    void modifyMenu();          // Modify_menu()
    void checkOut();            // Check_out()
    void searchGuest();         // Search_guest()
    void viewHistory();         // View_history()
    void applyCouponFlow();     // Apply_coupon()

    // Modify sub-features -------------------------------------------------
    void foodOrder(int room);   // Food_order()
    void modifyData(int room);  // Modify_data() + Retain()
    void laundryService(int room); // Laundry()
    void upgradeRoom(int room); // Upgrade_room()

    // Helpers -------------------------------------------------------------
    void printBooking(const Booking& b) const;      // Putdata()
    void collectGuests(Booking& b, int count, bool primary); // Getdata()
    int  promptRoomNumber();     // validated 1..100 with esc-to-exit (-1)
    std::vector<int> emptyRooms() const;
    std::vector<int> bookedRooms() const;

    Auth auth_;
    BookingRepository records_;  // active bookings
    BookingRepository history_;  // checkout history
};

#endif // HOTELAPP_HPP
