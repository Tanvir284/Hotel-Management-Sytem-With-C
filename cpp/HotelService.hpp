#ifndef HOTELSERVICE_HPP
#define HOTELSERVICE_HPP

#include "Auth.hpp"
#include "Booking.hpp"
#include "BookingRepository.hpp"

#include <string>
#include <vector>

// Non-interactive business layer shared by the console app and desktop GUI.
class HotelService {
public:
    HotelService();

    Auth& auth() { return auth_; }
    const Auth& auth() const { return auth_; }

    std::vector<Booking> activeBookings() const;
    std::vector<Booking> checkoutHistory() const;
    std::vector<int> bookedRooms() const;
    std::vector<int> emptyRooms() const;
    bool isOccupied(int room) const;
    bool findBooking(int room, Booking& out) const;

    void saveBooking(const Booking& booking);
    bool checkout(int room, Booking& checkedOut);
    std::vector<Booking> searchGuests(const std::string& query) const;

    bool applyCoupon(int room, const std::string& code, int& discountOut);
    bool assignRooms(int totalGuests, bool wantAC, int period,
                     std::vector<int>& roomsOut) const;

    bool addFoodOrder(int room, int itemIndex1Based, int qty, int& amountOut);
    bool modifyGuest(int room, int guestIndex1Based, const std::string& name,
                     const std::string& gender, int age, const std::string& address,
                     const std::string& phone, int periodDays);
    bool setLaundryFrequency(int room, int frequency);
    bool upgradeRoom(int fromRoom, int toRoom);

    static std::string roomFeaturesText(int room);
    static std::string writeReceipt(const Booking& b);
    static std::string roomSummary(int room);

private:
    Auth auth_;
    BookingRepository records_;
    BookingRepository history_;
};

#endif // HOTELSERVICE_HPP
