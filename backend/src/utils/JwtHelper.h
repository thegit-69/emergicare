/**
 * JwtHelper.h
 * Utility class for generating and verifying JSON Web Tokens.
 * Uses the jwt-cpp header-only library (fetched via CMake FetchContent).
 */

#pragma once

#include <string>
#include <optional>

// ---------------------------------------------------------------------------
// Claims extracted from a verified JWT token
// ---------------------------------------------------------------------------
struct JwtClaims {
    int         userId;
    std::string username;
    std::string role;     // "Admin" | "Doctor" | "Receptionist"
    std::string name;
};

// ---------------------------------------------------------------------------
// JwtHelper – static utility class
// ---------------------------------------------------------------------------
class JwtHelper {
public:
    /**
     * Generate a signed JWT token containing user identity claims.
     * Token expires in 7 days (matches original Node.js backend behaviour).
     */
    static std::string generateToken(int         userId,
                                     const std::string& username,
                                     const std::string& role,
                                     const std::string& name);

    /**
     * Verify a JWT token string.
     * Returns the decoded claims on success, or std::nullopt if the token
     * is invalid, expired, or has been tampered with.
     */
    static std::optional<JwtClaims> verifyToken(const std::string& token);

private:
    /** Read JWT_SECRET from environment (throws if not set). */
    static std::string getSecret();
};
