#ifndef BOOKINGREPOSITORY_HPP
#define BOOKINGREPOSITORY_HPP

#include "Booking.hpp"

#include <string>
#include <vector>

// Text-based persistence layer. Replaces the binary fread/fwrite of struct
// Hotel in the original C code with a line-per-record text file, and hides all
// file handling behind a small type-safe API (RAII fstreams internally).
class BookingRepository {
public:
    explicit BookingRepository(std::string path) : path_(std::move(path)) {}

    // Load all bookings. Corrupt lines are skipped defensively.
    std::vector<Booking> loadAll() const;

    // Overwrite the whole file with the given bookings.
    void saveAll(const std::vector<Booking>& bookings) const;

    // Append a single booking (used for new bookings and history log).
    void append(const Booking& booking) const;

    // Find a booking by room number. Returns true and fills `out` if found.
    bool findByRoom(int roomNo, Booking& out) const;

    // Returns true if a booking exists for the given room number.
    bool isOccupied(int roomNo) const;

    const std::string& path() const { return path_; }

private:
    std::string path_;
};

#endif // BOOKINGREPOSITORY_HPP
