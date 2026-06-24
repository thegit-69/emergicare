/**
 * UserController.cc
 * GET  /api/users  – list all staff (Admin only via RBAC in frontend)
 * POST /api/users  – create a new staff account (with bcrypt hash)
 */

#include "controllers/UserController.h"
#include "utils/PasswordHelper.h"

#include <drogon/drogon.h>
#include <json/json.h>

// ---------------------------------------------------------------------------
// GET /api/users – Return all users (id, username, role, name)
// ---------------------------------------------------------------------------
void UserController::getAll(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback) {

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT id, username, role, name FROM users ORDER BY id",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) {
                Json::Value u;
                u["id"]       = row["id"].as<int>();
                u["username"] = row["username"].as<std::string>();
                u["role"]     = row["role"].as<std::string>();
                u["name"]     = row["name"].as<std::string>();
                arr.append(u);
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch users.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

// ---------------------------------------------------------------------------
// POST /api/users – Create a new user account
// Body: { username, password, role, name }
// ---------------------------------------------------------------------------
void UserController::create(const drogon::HttpRequestPtr& req,
                             std::function<void(const drogon::HttpResponsePtr&)>&& callback) {

    auto body = req->getJsonObject();
    if (!body || !body->isMember("username") || !body->isMember("password") ||
        !body->isMember("role") || !body->isMember("name")) {
        Json::Value err; err["message"] = "username, password, role, and name are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp);
        return;
    }

    const std::string username = (*body)["username"].asString();
    const std::string password = (*body)["password"].asString();
    const std::string role     = (*body)["role"].asString();
    const std::string name     = (*body)["name"].asString();

    // Hash the password before storing
    std::string hashedPassword;
    try {
        hashedPassword = PasswordHelper::hash(password);
    } catch (const std::exception& e) {
        Json::Value err; err["message"] = "Failed to hash password.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k500InternalServerError);
        callback(resp);
        return;
    }

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "INSERT INTO users (username, password, role, name) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING id, username, role, name",

        [callback](const drogon::orm::Result& r) {
            const auto row = r[0];
            Json::Value u;
            u["id"]       = row["id"].as<int>();
            u["username"] = row["username"].as<std::string>();
            u["role"]     = row["role"].as<std::string>();
            u["name"]     = row["name"].as<std::string>();
            auto resp = drogon::HttpResponse::newHttpJsonResponse(u);
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },

        [callback](const drogon::orm::DrogonDbException& e) {
            std::string msg = e.base().what();
            Json::Value err;
            // PostgreSQL unique violation code = 23505
            if (msg.find("23505") != std::string::npos) {
                err["message"] = "Username already exists.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k409Conflict);
                callback(resp);
            } else {
                err["message"] = "Failed to create user.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                callback(resp);
            }
        },

        username, hashedPassword, role, name
    );
}
