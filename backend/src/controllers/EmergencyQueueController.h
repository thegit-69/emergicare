// EmergencyQueueController.h – CRUD for /api/emergency-queue
#pragma once
#include <drogon/HttpController.h>

class EmergencyQueueController : public drogon::HttpController<EmergencyQueueController> {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(EmergencyQueueController::getAll,  "/api/emergency-queue",     drogon::Get,    "AuthFilter");
    ADD_METHOD_TO(EmergencyQueueController::enqueue, "/api/emergency-queue",     drogon::Post,   "AuthFilter");
    ADD_METHOD_TO(EmergencyQueueController::update,  "/api/emergency-queue/{1}", drogon::Put,    "AuthFilter");
    ADD_METHOD_TO(EmergencyQueueController::dequeue, "/api/emergency-queue/{1}", drogon::Delete, "AuthFilter");
    METHOD_LIST_END

    // GET  /api/emergency-queue – Ordered queue (P1 first, then FIFO)
    void getAll(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // POST /api/emergency-queue – Add triaged patient to queue
    void enqueue(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback);

    // PUT  /api/emergency-queue/:id – Update status, assign doctor/bed
    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                int id);

    // DELETE /api/emergency-queue/:id – Remove from queue (discharged)
    void dequeue(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                 int id);
};
