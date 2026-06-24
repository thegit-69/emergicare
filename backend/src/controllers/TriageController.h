// TriageController.h – Full CRUD for /api/triage
// Demonstrates C++ function overloading via TriageService::assess()
#pragma once
#include <drogon/HttpController.h>

class TriageController : public drogon::HttpController<TriageController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(TriageController::getAll,   "/api/triage",     drogon::Get,    "AuthFilter");
    ADD_METHOD_TO(TriageController::create,   "/api/triage",     drogon::Post,   "AuthFilter");
    ADD_METHOD_TO(TriageController::getById,  "/api/triage/{1}", drogon::Get,    "AuthFilter");
    ADD_METHOD_TO(TriageController::update,   "/api/triage/{1}", drogon::Put,    "AuthFilter");
    METHOD_LIST_END

    // GET  /api/triage – All triage entries ordered by severity
    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // POST /api/triage – Assess and register patient (calls TriageService overloads)
    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // GET  /api/triage/:id – Single triage entry
    void getById(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                 int id);

    // PUT  /api/triage/:id – Update triage status
    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);
};
