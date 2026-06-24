// UserController.h – GET /api/users (Admin)  |  POST /api/users (create)
#pragma once
#include <drogon/HttpController.h>

class UserController : public drogon::HttpController<UserController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(UserController::getAll, "/api/users", drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(UserController::create, "/api/users", drogon::Post);   // no auth – setup endpoint
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
