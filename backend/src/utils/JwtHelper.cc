/**
 * JwtHelper.cc
 * Implementation of JWT token generation and verification.
 * Uses jwt-cpp (https://github.com/Thalhammer/jwt-cpp) – header-only,
 * fetched at build time via CMake FetchContent.
 */

#include "JwtHelper.h"

// jwt-cpp requires OpenSSL (linked via CMakeLists.txt)
#include <jwt-cpp/jwt.h>

#include <cstdlib>
#include <stdexcept>
#include <chrono>

// ---------------------------------------------------------------------------
// Private helper: read JWT_SECRET from environment
// ---------------------------------------------------------------------------
std::string JwtHelper::getSecret() {
    const char* secret = std::getenv("JWT_SECRET");
    if (!secret) {
        throw std::runtime_error("JWT_SECRET environment variable is not set");
    }
    return std::string(secret);
}

// ---------------------------------------------------------------------------
// Generate a signed HS256 JWT token
// ---------------------------------------------------------------------------
std::string JwtHelper::generateToken(int         userId,
                                     const std::string& username,
                                     const std::string& role,
                                     const std::string& name) {
    const auto secret  = getSecret();
    const auto now     = std::chrono::system_clock::now();
    const auto expires = now + std::chrono::hours(24 * 7); // 7 days

    return jwt::create()
        .set_type("JWT")
        .set_issued_at(now)
        .set_expires_at(expires)
        // Store user identity in payload claims
        .set_payload_claim("id",       jwt::claim(std::to_string(userId)))
        .set_payload_claim("username", jwt::claim(username))
        .set_payload_claim("role",     jwt::claim(role))
        .set_payload_claim("name",     jwt::claim(name))
        .sign(jwt::algorithm::hs256{secret});
}

// ---------------------------------------------------------------------------
// Verify a JWT token and extract claims
// ---------------------------------------------------------------------------
std::optional<JwtClaims> JwtHelper::verifyToken(const std::string& token) {
    try {
        const auto secret  = getSecret();
        const auto decoded = jwt::decode(token);

        // Verify algorithm, signature, and expiry
        jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_type("JWT")
            .verify(decoded);

        // Extract claims
        JwtClaims claims;
        claims.userId   = std::stoi(decoded.get_payload_claim("id").as_string());
        claims.username = decoded.get_payload_claim("username").as_string();
        claims.role     = decoded.get_payload_claim("role").as_string();
        claims.name     = decoded.get_payload_claim("name").as_string();

        return claims;

    } catch (const std::exception&) {
        // Token invalid, expired, or malformed — return empty optional
        return std::nullopt;
    }
}
