#pragma once
// Salted password hashing with PBKDF2-HMAC-SHA-256 (RFC 8018), using a recognised library:
//   Windows: Microsoft CNG (bcrypt.dll, ships with Windows)
//   Other:   OpenSSL
// A plain-text password is never stored (NFR-03). Only the random salt and the derived hash are.
#include <string>

class PasswordHasher {
public:
    // The default iteration count follows current OWASP guidance for PBKDF2-HMAC-SHA-256.
    // Tests pass a small number so they run quickly.
    explicit PasswordHasher(unsigned iterations = 600000) : iterations_(iterations) {}

    std::string newSalt() const;                                                    // 32 hex characters (128 random bits)
    std::string hash(const std::string& password, const std::string& salt) const;   // 64 hex characters
    bool verify(const std::string& password, const std::string& salt, const std::string& expectedHash) const;

private:
    unsigned iterations_;
};
