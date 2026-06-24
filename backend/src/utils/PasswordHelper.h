/**
 * PasswordHelper.h
 * Utility for bcrypt password hashing and verification.
 * Uses POSIX crypt_r() with the $2b$ (bcrypt) algorithm via libxcrypt.
 * Hashes produced are fully compatible with Node.js bcryptjs.
 */

#pragma once
#include <string>

class PasswordHelper {
public:
    /**
     * Hash a plain-text password using bcrypt (cost factor 10).
     * Returns the full bcrypt string (including $2b$10$ prefix + salt).
     */
    static std::string hash(const std::string& password);

    /**
     * Verify a plain-text password against a stored bcrypt hash.
     * Returns true if the password matches.
     */
    static bool verify(const std::string& password,
                       const std::string& storedHash);
};
