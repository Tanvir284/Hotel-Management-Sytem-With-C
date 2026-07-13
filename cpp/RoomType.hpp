#ifndef ROOMTYPE_HPP
#define ROOMTYPE_HPP

#include <memory>
#include <string>
#include <vector>

// --- Room category hierarchy -------------------------------------------------
// The original C code encoded room categories as magic number ranges scattered
// through Calc_Amount(), Room_Features(), Room_opt() and Room_stat(). Here the
// behaviour is centralised behind a polymorphic RoomType base class.

class RoomType {
public:
    virtual ~RoomType() = default;

    virtual std::string name() const = 0;   // e.g. "King/Queen"
    virtual int pricePerNight() const = 0;   // nightly room price
    virtual int capacity() const = 0;        // number of beds
    virtual bool isAC() const = 0;           // AC vs Non-AC
    virtual std::vector<std::string> features() const = 0;

    // Total room charge for a stay of `days` nights.
    int roomCharge(int days) const { return pricePerNight() * days; }
};

// Concrete categories -------------------------------------------------------

class KingQueenRoom : public RoomType {
public:
    std::string name() const override { return "King/Queen"; }
    int pricePerNight() const override { return 3000; }
    int capacity() const override { return 1; }
    bool isAC() const override { return true; }
    std::vector<std::string> features() const override {
        return {"Television & MP3 dock", "Free High speed Wi-Fi",
                "Remote control panels for lighting", "Refrigerator", "Geyser",
                "Wardrobes, iron and ironing board", "Tea/Coffee Maker",
                "Electric Safe", "Private Balcony", "Private bathroom"};
    }
};

class DeluxeDoubleRoom : public RoomType {
public:
    std::string name() const override { return "Deluxe Double"; }
    int pricePerNight() const override { return 2000; }
    int capacity() const override { return 2; }
    bool isAC() const override { return true; }
    std::vector<std::string> features() const override {
        return {"Television", "Free Wi-Fi", "Refrigerator", "Geyser"};
    }
};

class TripleRoom : public RoomType {
public:
    std::string name() const override { return "Triple Beds"; }
    int pricePerNight() const override { return 2700; }
    int capacity() const override { return 3; }
    bool isAC() const override { return true; }
    std::vector<std::string> features() const override {
        return {"Free Wi-Fi", "Geyser"};
    }
};

class QuadrupleRoom : public RoomType {
public:
    std::string name() const override { return "Quadruple Beds"; }
    int pricePerNight() const override { return 3600; }
    int capacity() const override { return 4; }
    bool isAC() const override { return true; }
    std::vector<std::string> features() const override {
        return {"Television", "Free Wi-Fi", "Refrigerator", "Geyser"};
    }
};

class StandardSingleRoom : public RoomType {
public:
    std::string name() const override { return "Standard Single"; }
    int pricePerNight() const override { return 1000; }
    int capacity() const override { return 1; }
    bool isAC() const override { return false; }
    std::vector<std::string> features() const override { return {}; }
};

class StandardTwinRoom : public RoomType {
public:
    std::string name() const override { return "Standard Twin"; }
    int pricePerNight() const override { return 1800; }
    int capacity() const override { return 2; }
    bool isAC() const override { return false; }
    std::vector<std::string> features() const override { return {}; }
};

#endif // ROOMTYPE_HPP
