#ifndef ROOMCATALOG_HPP
#define ROOMCATALOG_HPP

#include "RoomType.hpp"

#include <memory>
#include <stdexcept>

// Maps physical room numbers (1..100) to their category, preserving the exact
// ranges used by the original C code:
//   1-20  King/Queen (AC)      21-35 Deluxe Double (AC)
//   36-55 Triple (AC)          56-75 Quadruple (AC)
//   76-90 Standard Single (NAC)91-100 Standard Twin (NAC)
class RoomCatalog {
public:
    static constexpr int kMinRoom = 1;
    static constexpr int kMaxRoom = 100;

    static bool isValid(int room) { return room >= kMinRoom && room <= kMaxRoom; }

    // Returns a shared, immutable RoomType describing the given room number.
    // Throws std::out_of_range for invalid room numbers.
    static std::shared_ptr<const RoomType> typeOf(int room) {
        if (!isValid(room))
            throw std::out_of_range("Room number out of range: " + std::to_string(room));
        if (room <= 20) return king();
        if (room <= 35) return deluxe();
        if (room <= 55) return triple();
        if (room <= 75) return quad();
        if (room <= 90) return single();
        return twin();
    }

private:
    // RAII singletons: each category object is created once and shared.
    static std::shared_ptr<const RoomType> king() {
        static auto p = std::make_shared<const KingQueenRoom>();
        return p;
    }
    static std::shared_ptr<const RoomType> deluxe() {
        static auto p = std::make_shared<const DeluxeDoubleRoom>();
        return p;
    }
    static std::shared_ptr<const RoomType> triple() {
        static auto p = std::make_shared<const TripleRoom>();
        return p;
    }
    static std::shared_ptr<const RoomType> quad() {
        static auto p = std::make_shared<const QuadrupleRoom>();
        return p;
    }
    static std::shared_ptr<const RoomType> single() {
        static auto p = std::make_shared<const StandardSingleRoom>();
        return p;
    }
    static std::shared_ptr<const RoomType> twin() {
        static auto p = std::make_shared<const StandardTwinRoom>();
        return p;
    }
};

#endif // ROOMCATALOG_HPP
