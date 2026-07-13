#ifndef GUEST_HPP
#define GUEST_HPP

#include <string>
#include <utility>

// A single guest occupying a bed. In the original C code guests were stored as
// parallel arrays (Name[4][30], Gender[4][12], Age[4]) inside struct Hotel.
// Here they are a proper encapsulated value type using std::string.
class Guest {
public:
    Guest() = default;
    Guest(std::string name, std::string gender, int age)
        : name_(std::move(name)), gender_(std::move(gender)), age_(age) {}

    const std::string& name() const { return name_; }
    const std::string& gender() const { return gender_; }
    int age() const { return age_; }

    void setName(std::string name) { name_ = std::move(name); }
    void setGender(std::string gender) { gender_ = std::move(gender); }
    void setAge(int age) { age_ = age; }

    bool empty() const { return name_.empty(); }

private:
    std::string name_;
    std::string gender_;
    int age_ = 0;
};

#endif // GUEST_HPP
