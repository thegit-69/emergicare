// AppointmentController.h – GET/POST /api/appointments
#pragma once
#include <drogon/HttpController.h>

class AppointmentController : public drogon::HttpController<AppointmentController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AppointmentController::getAll, "/api/appointments", drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(AppointmentController::create, "/api/appointments", drogon::Post, "AuthFilter");
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
