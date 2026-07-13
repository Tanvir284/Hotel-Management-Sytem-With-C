#include "BookingRepository.hpp"

#include <fstream>

std::vector<Booking> BookingRepository::loadAll() const {
    std::vector<Booking> result;
    std::ifstream in(path_);
    if (!in) return result; // no file yet == no records
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        try {
            result.push_back(Booking::deserialize(line));
        } catch (const std::exception&) {
            // Skip corrupt line rather than aborting the whole load.
        }
    }
    return result;
}

void BookingRepository::saveAll(const std::vector<Booking>& bookings) const {
    std::ofstream out(path_, std::ios::trunc);
    for (const auto& b : bookings) out << b.serialize() << '\n';
}

void BookingRepository::append(const Booking& booking) const {
    std::ofstream out(path_, std::ios::app);
    out << booking.serialize() << '\n';
}

bool BookingRepository::findByRoom(int roomNo, Booking& out) const {
    for (auto& b : loadAll()) {
        if (b.roomNo() == roomNo) {
            out = b;
            return true;
        }
    }
    return false;
}

bool BookingRepository::isOccupied(int roomNo) const {
    Booking tmp;
    return findByRoom(roomNo, tmp);
}
