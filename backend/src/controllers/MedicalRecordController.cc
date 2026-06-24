/**
 * MedicalRecordController.cc – Medical record management
 * GET  /api/medical-records – list all records
 * POST /api/medical-records – add a new diagnosis/prescription record
 */

#include "controllers/MedicalRecordController.h"
#include <drogon/drogon.h>
#include <json/json.h>

static Json::Value rowToRecord(const drogon::orm::Row& row) {
    Json::Value r;
    r["id"]           = row["id"].as<int>();
    r["patientId"]    = row["patient_id"].as<int>();
    r["doctorName"]   = row["doctor_name"].as<std::string>();
    r["date"]         = row["date"].as<std::string>();
    r["diagnosis"]    = row["diagnosis"].as<std::string>();
    r["prescription"] = row["prescription"].as<std::string>();
    r["notes"]        = row["notes"].isNull() ? Json::Value("") : Json::Value(row["notes"].as<std::string>());
    r["createdAt"]    = row["created_at"].as<std::string>();
    return r;
}

void MedicalRecordController::getAll(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT id, patient_id, doctor_name, date::text, "
        "       diagnosis, prescription, notes, created_at::text "
        "FROM medical_records ORDER BY created_at DESC",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToRecord(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch medical records.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

void MedicalRecordController::create(const drogon::HttpRequestPtr& req,
                                      std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();
    if (!body) {
        Json::Value err; err["message"] = "Invalid JSON body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "INSERT INTO medical_records (patient_id, doctor_name, date, diagnosis, prescription, notes) "
        "VALUES ($1, $2, $3::date, $4, $5, $6) "
        "RETURNING id, patient_id, doctor_name, date::text, diagnosis, prescription, notes, created_at::text",

        [callback](const drogon::orm::Result& r) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(rowToRecord(r[0]));
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to add medical record.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        (*body)["patientId"].asInt(),
        (*body)["doctorName"].asString(),
        (*body)["date"].asString(),
        (*body)["diagnosis"].asString(),
        (*body)["prescription"].asString(),
        (*body).get("notes", "").asString()
    );
}
