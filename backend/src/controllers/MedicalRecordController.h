// MedicalRecordController.h – GET/POST /api/medical-records
#pragma once
#include <drogon/HttpController.h>

class MedicalRecordController : public drogon::HttpController<MedicalRecordController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(MedicalRecordController::getAll, "/api/medical-records", drogon::Get,  "AuthFilter");
    ADD_METHOD_TO(MedicalRecordController::create, "/api/medical-records", drogon::Post, "AuthFilter");
    METHOD_LIST_END

    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);
};
