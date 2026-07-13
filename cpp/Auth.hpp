#ifndef AUTH_HPP
#define AUTH_HPP

#include <string>

// Handles admin credential storage and login. Replaces login(),
// hash_password() and verify_password() from the C code. The password is
// stored as a hex-encoded XOR hash (identical scheme to the original, kept for
// data compatibility with existing admin.cfg files).
class Auth {
public:
    explicit Auth(std::string configFile) : configFile_(std::move(configFile)) {}

    // Interactive console login used by the terminal app. On first run (no
    // stored account) it walks the user through creating an admin account;
    // otherwise it prompts for credentials. Returns true on success.
    bool login() const;

    // Non-interactive helpers for GUI / automation.
    bool hasStoredAccount() const;
    bool createAccount(const std::string& user, const std::string& password);
    bool checkCredentials(const std::string& user, const std::string& password) const;

private:
    static constexpr int kHashKey = 7;

    static std::string hash(const std::string& input);
    bool verify(const std::string& input, const std::string& storedHash) const;
    bool loadCredentials(std::string& user, std::string& hash) const;
    void saveCredentials(const std::string& user, const std::string& hash) const;

    std::string configFile_;
};

#endif // AUTH_HPP
