#include "security/PasswordHasher.hpp"

#include <cstddef>
#include <stdexcept>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#else
#include <openssl/evp.h>
#include <openssl/rand.h>
#endif

namespace {

const std::size_t SALT_BYTES = 16;
const std::size_t HASH_BYTES = 32;

std::string toHex(const unsigned char* data, std::size_t size) {
    static const char digits[] = "0123456789abcdef";
    std::string text;
    text.reserve(size * 2);
    for (std::size_t i = 0; i < size; ++i) {
        text += digits[data[i] >> 4];
        text += digits[data[i] & 0x0F];
    }
    return text;
}

}  // namespace

std::string PasswordHasher::newSalt() const {
    unsigned char bytes[SALT_BYTES];
#ifdef _WIN32
    if (!BCRYPT_SUCCESS(BCryptGenRandom(nullptr, bytes, static_cast<ULONG>(SALT_BYTES), BCRYPT_USE_SYSTEM_PREFERRED_RNG))) {
        throw std::runtime_error("Could not generate a random salt");
    }
#else
    if (RAND_bytes(bytes, static_cast<int>(SALT_BYTES)) != 1) {
        throw std::runtime_error("Could not generate a random salt");
    }
#endif
    return toHex(bytes, SALT_BYTES);
}

std::string PasswordHasher::hash(const std::string& password, const std::string& salt) const {
    unsigned char derived[HASH_BYTES];
#ifdef _WIN32
    BCRYPT_ALG_HANDLE algorithm = nullptr;
    if (!BCRYPT_SUCCESS(BCryptOpenAlgorithmProvider(&algorithm, BCRYPT_SHA256_ALGORITHM, nullptr, BCRYPT_ALG_HANDLE_HMAC_FLAG))) {
        throw std::runtime_error("Could not open the password hashing provider");
    }
    const NTSTATUS status = BCryptDeriveKeyPBKDF2(
        algorithm, reinterpret_cast<PUCHAR>(const_cast<char*>(password.data())), static_cast<ULONG>(password.size()),
        reinterpret_cast<PUCHAR>(const_cast<char*>(salt.data())), static_cast<ULONG>(salt.size()), iterations_, derived,
        static_cast<ULONG>(HASH_BYTES), 0);
    BCryptCloseAlgorithmProvider(algorithm, 0);
    if (!BCRYPT_SUCCESS(status)) {
        throw std::runtime_error("Password hashing failed");
    }
#else
    if (PKCS5_PBKDF2_HMAC(password.data(), static_cast<int>(password.size()),
                          reinterpret_cast<const unsigned char*>(salt.data()), static_cast<int>(salt.size()),
                          static_cast<int>(iterations_), EVP_sha256(), static_cast<int>(HASH_BYTES), derived) != 1) {
        throw std::runtime_error("Password hashing failed");
    }
#endif
    return toHex(derived, HASH_BYTES);
}

bool PasswordHasher::verify(const std::string& password, const std::string& salt, const std::string& expectedHash) const {
    const std::string actual = hash(password, salt);
    if (actual.size() != expectedHash.size()) {
        return false;
    }
    // Compare every character, so the time taken does not reveal where the first difference is.
    unsigned char difference = 0;
    for (std::size_t i = 0; i < actual.size(); ++i) {
        difference |= static_cast<unsigned char>(actual[i] ^ expectedHash[i]);
    }
    return difference == 0;
}
