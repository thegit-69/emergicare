// PatientController.h – Full CRUD for /api/patients
#pragma once
#include <drogon/HttpController.h>

class PatientController : public drogon::HttpController<PatientController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(PatientController::getAll,  "/api/patients",     drogon::Get,    "AuthFilter");
    ADD_METHOD_TO(PatientController::create,  "/api/patients",     drogon::Post,   "AuthFilter");
    ADD_METHOD_TO(PatientController::update,  "/api/patients/{1}", drogon::Put,    "AuthFilter");
    ADD_METHOD_TO(PatientController::remove,  "/api/patients/{1}", drogon::Delete, "AuthFilter");
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);

    void remove(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);
};
