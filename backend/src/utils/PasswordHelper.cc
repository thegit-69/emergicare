/**
 * PasswordHelper.cc
 * bcrypt password hashing using POSIX crypt_r() provided by libxcrypt.
 *
 * libxcrypt on Ubuntu 22.04 supports the $2b$ bcrypt variant, making
 * hashes 100% compatible with passwords created by Node.js bcryptjs.
 *
 * Build dependency: link with -lcrypt (see CMakeLists.txt)
 */

#include "PasswordHelper.h"

#include <crypt.h>    // crypt_r(), struct crypt_data
#include <cstring>    // memset
#include <stdexcept>
#include <random>
#include <string>

// ---------------------------------------------------------------------------
// Internal helper: generate a bcrypt salt string
// Format: $2b$10$<22 base64 characters>
// The 22-char salt uses the bcrypt alphabet: [./A-Za-z0-9]
// ---------------------------------------------------------------------------
static std::string generateBcryptSalt() {
    // bcrypt base64 alphabet (different from standard base64)
    static const char alphabet[] =
        "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

    std::random_device rd;
    std::mt19937       gen(rd());
    std::uniform_int_distribution<int> dist(0, static_cast<int>(sizeof(alphabet) - 2));

    // Prefix: algorithm($2b$) + cost(10) + $
    std::string salt = "$2b$10$";
    salt.reserve(7 + 22);
    for (int i = 0; i < 22; ++i) {
        salt += alphabet[dist(gen)];
    }
    return salt;
}

// ---------------------------------------------------------------------------
// Hash a plain-text password with bcrypt (cost factor 10)
// ---------------------------------------------------------------------------
std::string PasswordHelper::hash(const std::string& password) {
    std::string salt = generateBcryptSalt();

    struct crypt_data data;
    std::memset(&data, 0, sizeof(data));

    char* result = crypt_r(password.c_str(), salt.c_str(), &data);
    if (!result || result[0] == '*') {
        throw std::runtime_error("PasswordHelper::hash() – bcrypt hashing failed");
    }
    return std::string(result);
}

// ---------------------------------------------------------------------------
// Verify a plain-text password against a stored bcrypt hash
// ---------------------------------------------------------------------------
bool PasswordHelper::verify(const std::string& password,
                            const std::string& storedHash) {
    if (storedHash.empty()) return false;

    struct crypt_data data;
    std::memset(&data, 0, sizeof(data));

    // crypt_r() uses the stored hash as the salt (it extracts the prefix
    // automatically), so re-hashing with the same salt produces the same hash.
    char* result = crypt_r(password.c_str(), storedHash.c_str(), &data);
    if (!result || result[0] == '*') return false;

    return storedHash == std::string(result);
}
