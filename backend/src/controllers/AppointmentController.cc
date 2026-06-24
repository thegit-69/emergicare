/**
 * AppointmentController.cc – Appointment management
 * GET  /api/appointments – list all, ordered by date DESC
 * POST /api/appointments – schedule a new appointment
 */

#include "controllers/AppointmentController.h"
#include <drogon/drogon.h>
#include <json/json.h>

static Json::Value rowToAppointment(const drogon::orm::Row& row) {
    Json::Value a;
    a["id"]          = row["id"].as<int>();
    a["patientId"]   = row["patient_id"].isNull() ? Json::Value() : Json::Value(row["patient_id"].as<int>());
    a["patientName"] = row["patient_name"].as<std::string>();
    a["doctorName"]  = row["doctor_name"].as<std::string>();
    a["date"]        = row["date"].as<std::string>();
    a["reason"]      = row["reason"].as<std::string>();
    a["status"]      = row["status"].as<std::string>();
    a["createdAt"]   = row["created_at"].as<std::string>();
    return a;
}

void AppointmentController::getAll(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT id, patient_id, patient_name, doctor_name, "
        "       date::text, reason, status, created_at::text "
        "FROM appointments ORDER BY date DESC",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToAppointment(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch appointments.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

void AppointmentController::create(const drogon::HttpRequestPtr& req,
                                    std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();
    if (!body) {
        Json::Value err; err["message"] = "Invalid JSON body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    const int         patientId   = (*body)["patientId"].asInt();
    const std::string patientName = (*body)["patientName"].asString();
    const std::string doctorName  = (*body)["doctorName"].asString();
    const std::string date        = (*body)["date"].asString();
    const std::string reason      = (*body)["reason"].asString();

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "INSERT INTO appointments (patient_id, patient_name, doctor_name, date, reason, status) "
        "VALUES ($1, $2, $3, $4::timestamp, $5, 'Scheduled') "
        "RETURNING id, patient_id, patient_name, doctor_name, date::text, reason, status, created_at::text",

        [callback](const drogon::orm::Result& r) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(rowToAppointment(r[0]));
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to schedule appointment.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        patientId, patientName, doctorName, date, reason
    );
}
