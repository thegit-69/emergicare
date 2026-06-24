// ResourceController.h – CRUD for /api/resources
#pragma once
#include <drogon/HttpController.h>

class ResourceController : public drogon::HttpController<ResourceController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ResourceController::getAll,       "/api/resources",           drogon::Get,    "AuthFilter");
    ADD_METHOD_TO(ResourceController::getAvailable, "/api/resources/available", drogon::Get,    "AuthFilter");
    ADD_METHOD_TO(ResourceController::create,       "/api/resources",           drogon::Post,   "AuthFilter");
    ADD_METHOD_TO(ResourceController::update,       "/api/resources/{1}",       drogon::Put,    "AuthFilter");
    METHOD_LIST_END

    // GET /api/resources – All resources
    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // GET /api/resources/available – Only available resources
    void getAvailable(const drogon::HttpRequestPtr& req,
                      std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // POST /api/resources – Add a new resource
    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // PUT /api/resources/:id – Allocate or release a resource
    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);
};
