#include "Booking.hpp"
#include "RoomCatalog.hpp"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace {
// Escape/unescape the field separator and newlines so free-text fields
// (names, addresses) can never corrupt the line-based record format.
std::string encode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char c : s) {
        switch (c) {
            case '\\': out += "\\\\"; break;
            case '|':  out += "\\p";  break;
            case '\n': out += "\\n";  break;
            case '\r': break;
            default:   out += c;      break;
        }
    }
    return out;
}

std::string decode(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            if (n == '\\') out += '\\';
            else if (n == 'p') out += '|';
            else if (n == 'n') out += '\n';
            else out += n;
        } else {
            out += s[i];
        }
    }
    return out;
}
} // namespace

int Booking::laundryCost() const {
    if (laundry_ <= 0) return 0; // defensive: never divide by zero
    return ((period_ + laundry_ - 1) / laundry_) * 500;
}

int Booking::subtotal() const {
    return roomPrice_ + laundryCost() + gymCost() + foodPrice_;
}

int Booking::total() const {
    return std::max(0, subtotal() - discount_);
}

void Booking::recomputeRoomPrice() {
    if (!RoomCatalog::isValid(roomNo_)) {
        roomPrice_ = 0;
        return;
    }
    roomPrice_ = RoomCatalog::typeOf(roomNo_)->roomCharge(period_);
}

std::string Booking::serialize() const {
    std::ostringstream os;
    os << roomNo_ << '|' << guests_.size();
    for (const auto& g : guests_) {
        os << '|' << encode(g.name()) << '|' << encode(g.gender()) << '|'
           << g.age();
    }
    os << '|' << encode(address_) << '|' << encode(phone_) << '|' << period_
       << '|' << day_ << '|' << month_ << '|' << year_ << '|' << hour_ << '|'
       << minute_ << '|' << payment_ << '|' << roomPrice_ << '|' << foodPrice_
       << '|' << laundry_ << '|' << gym_ << '|' << discount_;
    return os.str();
}

Booking Booking::deserialize(const std::string& line) {
    std::vector<std::string> f;
    std::string cur;
    std::istringstream is(line);
    while (std::getline(is, cur, '|')) f.push_back(cur);

    Booking b;
    size_t idx = 0;
    auto nextInt = [&]() -> int {
        if (idx >= f.size()) throw std::runtime_error("corrupt record");
        return std::stoi(f[idx++]);
    };
    auto nextStr = [&]() -> std::string {
        if (idx >= f.size()) throw std::runtime_error("corrupt record");
        return decode(f[idx++]);
    };

    b.roomNo_ = nextInt();
    int guestCount = nextInt();
    for (int i = 0; i < guestCount; ++i) {
        std::string name = nextStr();
        std::string gender = nextStr();
        int age = nextInt();
        b.guests_.emplace_back(name, gender, age);
    }
    b.address_ = nextStr();
    b.phone_ = nextStr();
    b.period_ = nextInt();
    b.day_ = nextInt();
    b.month_ = nextInt();
    b.year_ = nextInt();
    b.hour_ = nextInt();
    b.minute_ = nextInt();
    b.payment_ = nextInt();
    b.roomPrice_ = nextInt();
    b.foodPrice_ = nextInt();
    b.laundry_ = nextInt();
    b.gym_ = nextInt();
    if (idx < f.size()) b.discount_ = nextInt();
    return b;
}
