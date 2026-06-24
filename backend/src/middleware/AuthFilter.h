/**
 * AuthFilter.h
 * Drogon HTTP filter that validates JWT Bearer tokens.
 * Applied to all protected routes via the METHOD_LIST macros.
 *
 * On success: injects userId, username, role, name into request attributes.
 * On failure: returns 401/403 JSON error immediately, blocking the chain.
 */

#pragma once
#include <drogon/HttpFilter.h>

class AuthFilter : public drogon::HttpFilter<AuthFilter> {
public:
    void doFilter(const drogon::HttpRequestPtr&  req,
                  drogon::FilterCallback&&        fcb,
                  drogon::FilterChainCallback&&   fccb) override;
};
