#ifndef BOOKING_HPP
#define BOOKING_HPP

#include "Guest.hpp"

#include <algorithm>
#include <string>
#include <vector>

// Represents one room booking. This is the OOP replacement for the original
// global `struct Hotel H`. All pricing rules that were in Calc_Amount() and
// scattered checkout code are encapsulated here.
class Booking {
public:
    // --- Accessors ---------------------------------------------------------
    int roomNo() const { return roomNo_; }
    int capacity() const { return static_cast<int>(guests_.size()); }
    const std::vector<Guest>& guests() const { return guests_; }
    std::vector<Guest>& guests() { return guests_; }
    const std::string& address() const { return address_; }
    const std::string& phone() const { return phone_; }
    int period() const { return period_; }
    int laundry() const { return laundry_; }
    int gym() const { return gym_; }
    int foodPrice() const { return foodPrice_; }
    int payment() const { return payment_; }
    int roomPrice() const { return roomPrice_; }
    int discount() const { return discount_; }
    int checkInDay() const { return day_; }
    int checkInMonth() const { return month_; }
    int checkInYear() const { return year_; }
    int checkInHour() const { return hour_; }
    int checkInMinute() const { return minute_; }

    // --- Mutators ----------------------------------------------------------
    void setRoomNo(int r) { roomNo_ = r; }
    void addGuest(const Guest& g) { guests_.push_back(g); }
    void setAddress(std::string a) { address_ = std::move(a); }
    void setPhone(std::string p) { phone_ = std::move(p); }
    void setPeriod(int p) { period_ = p; }
    void setLaundry(int l) { laundry_ = l; }
    void setGym(int g) { gym_ = g; }
    void setFoodPrice(int f) { foodPrice_ = f; }
    void addFood(int f) { foodPrice_ += f; }
    void setPayment(int p) { payment_ = p; }
    void setRoomPrice(int p) { roomPrice_ = p; }
    void setDiscount(int d) { discount_ = std::max(0, d); }
    void addDiscount(int d) { discount_ = std::max(0, discount_ + d); }
    void setCheckIn(int day, int month, int year, int hour, int minute) {
        day_ = day; month_ = month; year_ = year; hour_ = hour; minute_ = minute;
    }

    // --- Pricing (encapsulated business rules) -----------------------------
    int laundryCost() const;   // laundry service charge for the whole stay
    int gymCost() const { return gym_ * 1000; }
    int subtotal() const;      // room + laundry + gym + food (before discounts)
    int total() const;         // subtotal minus any coupon discounts
    int balanceDue() const { return total() - payment_; }

    // Recomputes room price from the room number & period (was Calc_Amount()).
    void recomputeRoomPrice();

    // --- Text serialization (one record per line, '|' separated) -----------
    std::string serialize() const;
    static Booking deserialize(const std::string& line);

private:
    int roomNo_ = 0;
    std::vector<Guest> guests_;
    std::string address_;
    std::string phone_;
    int period_ = 0;
    int day_ = 0, month_ = 0, year_ = 0;
    int hour_ = 0, minute_ = 0;
    int payment_ = 0;
    int roomPrice_ = 0;
    int foodPrice_ = 0;
    int laundry_ = 1;
    int gym_ = 0;
    int discount_ = 0;
};

#endif // BOOKING_HPP
