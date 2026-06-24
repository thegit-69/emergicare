/**
 * AuthFilter.cc
 * JWT Bearer token validation middleware.
 *
 * Flow:
 *  1. Extract "Authorization: Bearer <token>" header
 *  2. Verify token signature + expiry via JwtHelper
 *  3. If valid → inject claims into request attributes and call fccb()
 *  4. If invalid → call fcb() with 401/403 error response (stops chain)
 */

#include "middleware/AuthFilter.h"
#include "utils/JwtHelper.h"

#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

void AuthFilter::doFilter(const drogon::HttpRequestPtr&  req,
                          drogon::FilterCallback&&        fcb,
                          drogon::FilterChainCallback&&   fccb) {

    // --------------------------------------------------------
    // 1. Extract Bearer token from Authorization header
    // --------------------------------------------------------
    const std::string authHeader = req->getHeader("authorization");

    if (authHeader.empty() || authHeader.size() < 8 ||
        authHeader.substr(0, 7) != "Bearer ") {

        Json::Value body;
        body["success"] = false;
        body["message"] = "Access token is required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(drogon::k401Unauthorized);
        fcb(resp);
        return;
    }

    const std::string token = authHeader.substr(7);

    // --------------------------------------------------------
    // 2. Verify the token
    // --------------------------------------------------------
    auto claims = JwtHelper::verifyToken(token);

    if (!claims.has_value()) {
        Json::Value body;
        body["success"] = false;
        body["message"] = "Invalid or expired token.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(drogon::k403Forbidden);
        fcb(resp);
        return;
    }

    // --------------------------------------------------------
    // 3. Inject user context into request attributes
    //    Controllers read these via req->getAttributes()->get<T>()
    // --------------------------------------------------------
    req->attributes()->insert("userId",   claims->userId);
    req->attributes()->insert("username", claims->username);
    req->attributes()->insert("role",     claims->role);
    req->attributes()->insert("name",     claims->name);

    // --------------------------------------------------------
    // 4. Continue to the next filter / controller
    // --------------------------------------------------------
    fccb();
}
