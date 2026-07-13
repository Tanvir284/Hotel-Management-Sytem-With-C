#include "Auth.hpp"
#include "Terminal.hpp"

#include <cstdio>
#include <fstream>
#include <iostream>
#include <sstream>

std::string Auth::hash(const std::string& input) {
    std::ostringstream os;
    for (unsigned char ch : input) {
        unsigned char v = ch ^ kHashKey;
        char buf[3];
        std::snprintf(buf, sizeof(buf), "%02X", v);
        os << buf;
    }
    return os.str();
}

bool Auth::verify(const std::string& input, const std::string& storedHash) const {
    return hash(input) == storedHash;
}

bool Auth::loadCredentials(std::string& user, std::string& h) const {
    std::ifstream in(configFile_);
    if (!in) return false;
    return static_cast<bool>(in >> user >> h);
}

void Auth::saveCredentials(const std::string& user, const std::string& h) const {
    std::ofstream out(configFile_, std::ios::trunc);
    out << user << ' ' << h << '\n';
}

bool Auth::hasStoredAccount() const {
    std::string user, h;
    return loadCredentials(user, h);
}

bool Auth::createAccount(const std::string& user, const std::string& password) {
    if (user.empty()) return false;
    saveCredentials(user, hash(password));
    return true;
}

bool Auth::checkCredentials(const std::string& user, const std::string& password) const {
    std::string storedUser, storedHash;
    if (!loadCredentials(storedUser, storedHash)) return false;
    return user == storedUser && verify(password, storedHash);
}

bool Auth::login() const {
    // First run: no admin account exists yet, so guide the user to create one.
    if (!hasStoredAccount()) {
        std::cout << "\n\t\t No admin account found. Create one to continue.\n";
        std::cout << "\n Choose admin username: ";
        std::string user = Terminal::readLine();
        if (user.empty()) {
            std::cout << "\n Username cannot be empty.\n";
            Terminal::pause();
            return false;
        }
        std::cout << " Choose admin password: ";
        std::string password = Terminal::readPassword();
        saveCredentials(user, hash(password));
        std::cout << "\n\n Admin account created successfully. Please log in.\n";
        Terminal::pause();
    }

    // Standard login flow: three attempts before giving up.
    for (int attempt = 1; attempt <= 3; ++attempt) {
        std::cout << "\n\t\t\t\t     ADMIN LOGIN\n";
        std::cout << "\n Username: ";
        std::string user = Terminal::readLine();
        std::cout << " Password: ";
        std::string password = Terminal::readPassword();

        if (checkCredentials(user, password)) {
            std::cout << "\n\n Login successful. Welcome, " << user << "!\n";
            Terminal::pause();
            return true;
        }
        std::cout << "\n\n Invalid credentials. Attempts left: " << (3 - attempt) << "\n";
        Terminal::pause();
        Terminal::clear();
    }
    return false;
}
