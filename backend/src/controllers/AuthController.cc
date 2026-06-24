/**
 * AuthController.cc – Login endpoint
 * POST /api/login
 *
 * 1. Parse username + password from request body
 * 2. Fetch user row from PostgreSQL
 * 3. Verify bcrypt password hash
 * 4. On success: generate JWT and return {success, token, user}
 */

#include "controllers/AuthController.h"
#include "utils/JwtHelper.h"
#include "utils/PasswordHelper.h"

#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

void AuthController::login(const drogon::HttpRequestPtr& req,
                            std::function<void(const drogon::HttpResponsePtr&)>&& callback) {

    // --- Parse request body ---
    auto body = req->getJsonObject();
    if (!body || !body->isMember("username") || !body->isMember("password")) {
        Json::Value resp;
        resp["success"] = false;
        resp["message"] = "Username and password are required.";
        auto response = drogon::HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    const std::string username = (*body)["username"].asString();
    const std::string password = (*body)["password"].asString();

    if (username.empty() || password.empty()) {
        Json::Value resp;
        resp["success"] = false;
        resp["message"] = "Username and password cannot be empty.";
        auto response = drogon::HttpResponse::newHttpJsonResponse(resp);
        response->setStatusCode(drogon::k400BadRequest);
        callback(response);
        return;
    }

    // --- Query database (async) ---
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT id, username, password, role, name FROM users WHERE username = $1",

        // Success callback
        [callback, password](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value resp;
                resp["success"] = false;
                resp["message"] = "Invalid credentials.";
                auto response = drogon::HttpResponse::newHttpJsonResponse(resp);
                response->setStatusCode(drogon::k401Unauthorized);
                callback(response);
                return;
            }

            const auto row        = r[0];
            const std::string storedHash = row["password"].as<std::string>();

            // Verify bcrypt hash (PasswordHelper::verify uses crypt_r)
            if (!PasswordHelper::verify(password, storedHash)) {
                Json::Value resp;
                resp["success"] = false;
                resp["message"] = "Invalid credentials.";
                auto response = drogon::HttpResponse::newHttpJsonResponse(resp);
                response->setStatusCode(drogon::k401Unauthorized);
                callback(response);
                return;
            }

            // Build user payload (never include password in response)
            const int         userId   = row["id"].as<int>();
            const std::string uname    = row["username"].as<std::string>();
            const std::string role     = row["role"].as<std::string>();
            const std::string name     = row["name"].as<std::string>();

            const std::string token = JwtHelper::generateToken(userId, uname, role, name);

            Json::Value user;
            user["id"]       = userId;
            user["username"] = uname;
            user["role"]     = role;
            user["name"]     = name;

            Json::Value resp;
            resp["success"] = true;
            resp["token"]   = token;
            resp["user"]    = user;

            auto response = drogon::HttpResponse::newHttpJsonResponse(resp);
            callback(response);
        },

        // Database error callback
        [callback](const drogon::orm::DrogonDbException& e) {
            Json::Value resp;
            resp["success"] = false;
            resp["message"] = "Database error during login.";
            auto response = drogon::HttpResponse::newHttpJsonResponse(resp);
            response->setStatusCode(drogon::k500InternalServerError);
            callback(response);
        },

        username // $1 parameter
    );
}
