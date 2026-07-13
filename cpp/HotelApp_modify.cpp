#include "HotelApp.hpp"
#include "DateUtil.hpp"
#include "RoomCatalog.hpp"
#include "Terminal.hpp"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

namespace {
char readChar3() {
    std::string line = Terminal::readLine();
    for (char c : line)
        if (!std::isspace(static_cast<unsigned char>(c))) return c;
    return '\0';
}

std::string toLower3(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return s;
}

// Replace a booking (matched by room number) inside the record file.
void updateBooking(BookingRepository& repo, const Booking& updated) {
    std::vector<Booking> all = repo.loadAll();
    for (auto& b : all)
        if (b.roomNo() == updated.roomNo()) b = updated;
    repo.saveAll(all);
}
} // namespace

// --- Modify menu (was Modify_menu) -----------------------------------------
void HotelApp::modifyMenu() {
    Terminal::setColor("1E");
    int choice = 0;
    do {
        Terminal::clear();
        std::cout << "\n\n\n\t\t\t\t\t  ***********";
        std::cout << "\n\t\t\t\t\t  MODIFY MENU";
        std::cout << "\n\t\t\t\t\t  ***********\n\n";
        std::cout << "\n\t\t\t\t\t 1. Food Order\n";
        std::cout << "\n\t\t\t\t\t 2. Modify Customer Record\n";
        std::cout << "\n\t\t\t\t\t 3. Modify Laundry Service\n";
        std::cout << "\n\t\t\t\t\t 4. Upgrade Room\n";
        std::cout << "\n\t\t\t\t\t 5. Go Back to Main Menu\n";
        choice = Terminal::readInt("\n\n\n\t\t\t\t\t Enter your choice:  ");
        if (choice < 1 || choice > 5)
            Terminal::pause("\n\n Wrong Choice!! Enter a valid choice.");
    } while (choice < 1 || choice > 5);

    if (choice == 5) return;

    Terminal::clear();
    int r = promptRoomNumber();
    if (r == -1) return;

    switch (choice) {
        case 1: foodOrder(r); break;
        case 2: modifyData(r); break;
        case 3: laundryService(r); break;
        case 4: upgradeRoom(r); break;
    }
}

// --- Food order (was Food_order) -------------------------------------------
void HotelApp::foodOrder(int room) {
    static const std::vector<std::string> food = {
        "SHAHI PANEER", "KADAI PANEER", "CHEESE KORMA", "MALAI KOFTA",
        "MATAR PANEER", "PALAK PANEER", "MIX VEGITABLES", "RAJMA MAKHANI",
        "DAL MAKHANI", "SALAD(GREEN)", "DUM ALOO", "MUSHROOM PANEER",
        "MASALA DOSA", "MASALA CHICKEN", "DAHI VADA", "MUTTON KORMA",
        "MUTTON CURRY", "EGG CURRY", "BUTTER CHICKEN", "FISH CURRY",
        "CHICKEN CURRY", "CHILLI CHICKEN", "CHICKEN TIKKA", "SEEKH KABAB",
        "CHICKEN TANDORI", "PANEER TIKKA", "CHICKEN SEEKHKABAB",
        "CHICKEN BIRYANI", "PANEER PULAO", "VEG.PULAO", "RUMALI ROTI",
        "ALOO PARATHA", "PANEER PARANTHA", "PIZZA MARGHERITA", "VEG.BURGER",
        "CHICKEN BURGER", "PANEER BURGER", "CHEESE SANDWICH", "TEA", "COFFEE",
        "COLD COFFEE", "LASSI", "HOT CHOCOLATE", "SOFT DRINKS", "COLD DRINKS",
        "STRAWBERRY", "CHOCOLATE", "BLACK FOREST", "VANILLA", "PISTA BADAM"};
    static const std::vector<int> price = {
        200, 220, 160, 130, 125, 110, 150, 160, 140, 100, 110, 180, 120,
        80,  100, 250, 300, 150, 240, 200, 260, 250, 150, 230, 240, 160,
        260, 250, 220, 200, 40,  30,  35,  300, 200, 240, 220, 150, 20,
        20,  30,  30,  50,  50,  60,  30,  40,  60,  30,  40};

    Booking b;
    if (!records_.findByRoom(room, b)) {
        Terminal::pause("\n Sorry the Room is vacant.");
        return;
    }

    int totalPrice = 0, count = 0;
    while (true) {
        Terminal::clear();
        std::cout << "\n\t\t MENU CARD\n";
        for (size_t i = 0; i < food.size(); ++i) {
            std::cout << (i + 1) << "." << food[i] << " " << price[i] << "/-\t";
            if ((i + 1) % 4 == 0) std::cout << "\n";
        }
        int choice = Terminal::readInt("\n\n Enter Item No. of Food: ");
        if (choice < 1 || choice > static_cast<int>(food.size())) {
            std::cout << " Wrong Input! Returning.\n";
            break;
        }
        int qty = Terminal::readInt("\n Enter Quantity of '" +
                                    food[choice - 1] + "': ");
        if (qty < 0) qty = 0;
        int amount = price[choice - 1] * qty;
        totalPrice += amount;
        ++count;
        std::cout << " Order " << count << " Amount = " << amount;
        std::cout << "\n Order 1 more Food Item (Y/N): ";
        char c = readChar3();
        if (!(c == 'Y' || c == 'y')) break;
    }

    std::cout << "\n Total Amount of " << count << " foods: " << totalPrice
              << "/-\n";
    b.addFood(totalPrice);
    updateBooking(records_, b);
    std::cout << " Food Price of room no " << room << " has been updated.";
    Terminal::pause();
}

// --- Modify customer data (was Modify_data + Retain) -----------------------
void HotelApp::modifyData(int room) {
    Booking b;
    if (!records_.findByRoom(room, b)) {
        Terminal::pause("\n Sorry! the Room is vacant!!");
        return;
    }
    printBooking(b);

    int n = Terminal::readInt(
        "\n\n Enter Guest No. whose data is to be modified: ");
    if (n < 1 || n > b.capacity()) {
        Terminal::pause("\n Invalid guest number.");
        return;
    }
    Guest& g = b.guests()[n - 1];

    std::cout << "\n Enter New details (press '0' to retain old value)\n";
    std::cout << " Name : ";
    std::string name = Terminal::readLine();
    if (name != "0" && !name.empty()) g.setName(name);

    while (true) {
        std::cout << " Gender [M/F/T]: ";
        char gd = readChar3();
        if (gd == 'M' || gd == 'm') { g.setGender("Male"); break; }
        if (gd == 'F' || gd == 'f') { g.setGender("Female"); break; }
        if (gd == 'T' || gd == 't') { g.setGender("Transgender"); break; }
        if (gd == '0' || gd == '\0') break;
        std::cout << " Incorrect option\n";
    }

    int age = Terminal::readInt(" Age: ");
    if (age != 0) g.setAge(age);

    if (!b.address().empty()) {
        std::cout << " Address: ";
        std::string a = Terminal::readLine();
        if (a != "0" && !a.empty()) b.setAddress(a);
        std::cout << " Phone no: ";
        std::string p = Terminal::readLine();
        if (p != "0" && !p.empty()) b.setPhone(p);
    }

    int period = -1;
    do {
        period = Terminal::readInt(" Duration (0 to retain): ");
        if (period > 60) std::cout << " Stay cannot exceed 60 Days.\n";
    } while (period < 0 || period > 60);
    if (period != 0) {
        b.setPeriod(period);
        if (b.gym() > period) b.setGym(period);
        b.recomputeRoomPrice(); // keep the bill consistent (bug fix vs C code)
    }

    updateBooking(records_, b);
    std::cout << "\n Record of room no " << room << " has been modified!!";
    Terminal::pause();
}

// --- Laundry service (was Laundry) -----------------------------------------
void HotelApp::laundryService(int room) {
    Booking b;
    if (!records_.findByRoom(room, b)) {
        Terminal::pause("\n Sorry! the Room is vacant!!");
        return;
    }
    std::cout << "\n Current frequency of Laundry service is: " << b.laundry();
    std::cout << "\n Modify Laundry frequency (Y/N): ";
    char c = readChar3();
    if (c == 'Y' || c == 'y') {
        int l = 0;
        do {
            l = Terminal::readInt("\n Enter New Frequency (1-4): ");
            if (l < 1 || l > 4) std::cout << " Enter 1 to 4.\n";
        } while (l < 1 || l > 4);
        b.setLaundry(l);
        b.recomputeRoomPrice();
        updateBooking(records_, b);
        std::cout << "\n Laundry Frequency of Room " << room << " modified.";
    }
    Terminal::pause();
}

// --- Upgrade / move room (was Upgrade_room) --------------------------------
void HotelApp::upgradeRoom(int room) {
    Booking b;
    if (!records_.findByRoom(room, b)) {
        Terminal::pause("\n Room is vacant or does not exist.");
        return;
    }
    std::cout << "\n Current Room: " << b.roomNo();
    std::cout << "\n Guests: " << b.capacity();
    std::cout << "\n Current Total Amount: " << b.total() << "\n";

    int newRoom = Terminal::readInt("\n Enter new room number to move guest to: ");
    if (!RoomCatalog::isValid(newRoom)) {
        Terminal::pause("\n Invalid room number.");
        return;
    }
    if (records_.isOccupied(newRoom)) {
        Terminal::pause("\n That room is already occupied!");
        return;
    }

    // Replace the record identified by its OLD room number, then change it.
    std::vector<Booking> all = records_.loadAll();
    for (auto& rec : all) {
        if (rec.roomNo() == room) {
            rec.setRoomNo(newRoom);
            rec.recomputeRoomPrice();
            b = rec;
            break;
        }
    }
    records_.saveAll(all);

    std::cout << "\n Room upgraded successfully from " << room << " to "
              << newRoom << "!";
    std::cout << "\n New Total Amount: " << b.total() << "\n";
    Terminal::pause();
}

// --- Checkout (was Check_out) ----------------------------------------------
void HotelApp::checkOut() {
    Terminal::setColor("CF");
    int r = promptRoomNumber();
    if (r == -1) return;

    std::vector<Booking> all = records_.loadAll();
    auto it = std::find_if(all.begin(), all.end(),
                           [r](const Booking& b) { return b.roomNo() == r; });
    if (it == all.end()) {
        Terminal::pause("\n Sorry the room is vacant!!");
        return;
    }

    Booking b = *it;
    printBooking(b);
    std::cout << "\n\n Check-out this Guest (y/n): ";
    char c = readChar3();
    if (!(c == 'y' || c == 'Y')) {
        Terminal::pause("\n Check-out cancelled.");
        return;
    }

    std::cout << "\n Price of Room Booking for " << b.period()
              << " Days is " << b.roomPrice();
    if (b.foodPrice() > 0)
        std::cout << "\n Price for Food ordered is " << b.foodPrice();
    std::cout << "\n Price for Laundry Service is " << b.laundryCost();
    if (b.gym() > 0)
        std::cout << "\n Price of Fitness center for " << b.gym()
                  << " Days is " << b.gymCost();
    std::cout << "\n\n Total Expense of Room is: " << b.total();
    std::cout << "\n Amount Payable (Remaining): " << b.balanceDue();

    // Write a receipt file.
    std::string receipt = "Receipt_Room" + std::to_string(r) + ".txt";
    std::ofstream rc(receipt);
    if (rc) {
        rc << "========================================\n";
        rc << "       THE RUHIT'S PALACE - RECEIPT\n";
        rc << "========================================\n\n";
        rc << " Room No: " << b.roomNo() << "\n";
        rc << " Guest Name: " << (b.capacity() > 0 ? b.guests()[0].name() : "") << "\n";
        rc << " Check-in: " << b.checkInDay() << "/" << b.checkInMonth() << "/"
           << b.checkInYear() << " at " << b.checkInHour() << ":"
           << b.checkInMinute() << "\n";
        rc << " Duration: " << b.period() << " days\n\n";
        rc << " Room Charges:    " << b.roomPrice() << "\n";
        rc << " Food Charges:    " << b.foodPrice() << "\n";
        rc << " Laundry:         " << b.laundryCost() << "\n";
        rc << " Gym:             " << b.gymCost() << "\n";
        rc << "----------------------------------------\n";
        rc << " TOTAL:           " << b.total() << "\n";
        rc << " Advance Paid:    " << b.payment() << "\n";
        rc << " BALANCE DUE:     " << b.balanceDue() << "\n";
        rc << "========================================\n";
        rc << "         Thank you for your stay!\n";
        std::cout << "\n\n Receipt saved to " << receipt;
    }

    history_.append(b);       // log to history
    all.erase(it);            // remove from active records
    records_.saveAll(all);
    std::cout << "\n Record of room no " << r << " successfully deleted\n";
    Terminal::pause();
}

// --- Search guest by name (was Search_guest) -------------------------------
void HotelApp::searchGuest() {
    Terminal::clear();
    std::cout << "\n Search Guest by Name\n ********************\n\n";
    std::cout << " Enter Guest Name (partial matches allowed): ";
    std::string query = toLower3(Terminal::readLine());
    if (query.empty()) return;

    int found = 0;
    for (const auto& b : records_.loadAll()) {
        int idx = -1;
        for (int i = 0; i < b.capacity(); ++i) {
            if (toLower3(b.guests()[i].name()).find(query) != std::string::npos) {
                idx = i;
                break;
            }
        }
        if (idx != -1) {
            ++found;
            std::cout << "\n-------------------------------------------------";
            std::cout << "\n Room No: " << b.roomNo();
            std::cout << "\n Guest Name: " << b.guests()[idx].name();
            std::cout << "\n Phone: " << b.phone();
            std::cout << "\n Check-in: " << b.checkInDay() << "/"
                      << b.checkInMonth() << "/" << b.checkInYear();
        }
    }
    if (found == 0)
        std::cout << "\n No guests found matching '" << query << "'.\n";
    else
        std::cout << "\n\n Total " << found << " match(es) found.\n";
    Terminal::pause("\n Press any key to return to main menu.");
}

// --- Guest history (was View_history) --------------------------------------
void HotelApp::viewHistory() {
    Terminal::clear();
    std::cout << "\n Guest Checkout History\n **********************\n\n";
    auto all = history_.loadAll();
    if (all.empty()) {
        Terminal::pause("\n No past checkout history found.");
        return;
    }
    for (const auto& b : all) {
        std::cout << "\n Room " << b.roomNo() << " | "
                  << (b.capacity() > 0 ? b.guests()[0].name() : "")
                  << " | Stayed: " << b.period()
                  << " days | Total Billed: " << b.total();
    }
    std::cout << "\n\n Total Past Guests: " << all.size();
    Terminal::pause("\n\n Press any key to return to main menu.");
}

// --- Apply coupon (was Apply_coupon) ---------------------------------------
void HotelApp::applyCouponFlow() {
    std::cout << "\n Enter coupon code: ";
    std::string code = Terminal::readLine();
    int r = Terminal::readInt(" Enter room number to apply discount: ");

    int percent = 0;
    if (code == "WELCOME10") percent = 10;
    else if (code == "STAY20") percent = 20;
    else {
        std::cout << "\n Invalid or expired coupon code: '" << code << "'\n";
        Terminal::pause();
        return;
    }

    Booking b;
    if (!records_.findByRoom(r, b)) {
        std::cout << "\n Room " << r << " is vacant or does not exist.\n";
        Terminal::pause();
        return;
    }

    int discount = (b.total() * percent) / 100;
    b.addDiscount(discount);
    updateBooking(records_, b);
    std::cout << "\n Coupon '" << code << "' applied successfully!";
    std::cout << "\n You saved " << discount << "/- (New Total: " << b.total()
              << "/-)\n";
    Terminal::pause();
}
