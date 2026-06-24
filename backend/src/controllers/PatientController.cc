/**
 * PatientController.cc – Patient CRUD operations
 * Matches original Node.js API shape exactly for frontend compatibility.
 */

#include "controllers/PatientController.h"
#include <drogon/drogon.h>
#include <json/json.h>

// Helper: build a JSON patient object from a DB row
static Json::Value rowToPatient(const drogon::orm::Row& row) {
    Json::Value p;
    p["id"]        = row["id"].as<int>();
    p["name"]      = row["name"].as<std::string>();
    p["dob"]       = row["dob"].isNull()     ? Json::Value() : Json::Value(row["dob"].as<std::string>());
    p["gender"]    = row["gender"].isNull()  ? Json::Value() : Json::Value(row["gender"].as<std::string>());
    p["contact"]   = row["contact"].isNull() ? Json::Value() : Json::Value(row["contact"].as<std::string>());
    p["email"]     = row["email"].isNull()   ? Json::Value() : Json::Value(row["email"].as<std::string>());
    p["address"]   = row["address"].isNull() ? Json::Value() : Json::Value(row["address"].as<std::string>());
    p["createdAt"] = row["created_at"].isNull() ? Json::Value() : Json::Value(row["created_at"].as<std::string>());
    return p;
}

// ---------------------------------------------------------------------------
// GET /api/patients
// ---------------------------------------------------------------------------
void PatientController::getAll(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT id, name, dob::text, gender, contact, email, address, created_at::text "
        "FROM patients ORDER BY id",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToPatient(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch patients.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

// ---------------------------------------------------------------------------
// POST /api/patients
// ---------------------------------------------------------------------------
void PatientController::create(const drogon::HttpRequestPtr& req,
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
        "INSERT INTO patients (name, dob, gender, contact, email, address) "
        "VALUES ($1, $2::date, $3, $4, $5, $6) "
        "RETURNING id, name, dob::text, gender, contact, email, address, created_at::text",

        [callback](const drogon::orm::Result& r) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(rowToPatient(r[0]));
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to create patient.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        (*body)["name"].asString(),
        (*body)["dob"].asString(),
        (*body)["gender"].asString(),
        (*body)["contact"].asString(),
        (*body)["email"].asString(),
        (*body)["address"].asString()
    );
}

// ---------------------------------------------------------------------------
// PUT /api/patients/:id
// ---------------------------------------------------------------------------
void PatientController::update(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                int id) {
    auto body = req->getJsonObject();
    if (!body) {
        Json::Value err; err["message"] = "Invalid JSON body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "UPDATE patients SET name=$1, dob=$2::date, gender=$3, contact=$4, email=$5, address=$6 "
        "WHERE id=$7 "
        "RETURNING id, name, dob::text, gender, contact, email, address, created_at::text",

        [callback](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err; err["message"] = "Patient not found.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(rowToPatient(r[0])));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to update patient.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        (*body)["name"].asString(),
        (*body)["dob"].asString(),
        (*body)["gender"].asString(),
        (*body)["contact"].asString(),
        (*body)["email"].asString(),
        (*body)["address"].asString(),
        id
    );
}

// ---------------------------------------------------------------------------
// DELETE /api/patients/:id
// Also deletes associated records (cascade handled by DB constraints)
// ---------------------------------------------------------------------------
void PatientController::remove(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                int id) {
    auto db = drogon::app().getDbClient();

    // Delete appointments first (no cascade constraint on patient_name column)
    db->execSqlAsync(
        "DELETE FROM appointments WHERE patient_id = $1",
        [db, callback, id](const drogon::orm::Result&) {
            // Then delete the patient (medical_records cascade via FK)
            db->execSqlAsync(
                "DELETE FROM patients WHERE id = $1",
                [callback](const drogon::orm::Result& r) {
                    if (r.affectedRows() == 0) {
                        Json::Value err; err["message"] = "Patient not found.";
                        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                        resp->setStatusCode(drogon::k404NotFound);
                        callback(resp); return;
                    }
                    Json::Value ok; ok["success"] = true; ok["message"] = "Patient deleted successfully.";
                    callback(drogon::HttpResponse::newHttpJsonResponse(ok));
                },
                [callback](const drogon::orm::DrogonDbException&) {
                    Json::Value err; err["message"] = "Failed to delete patient.";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                    resp->setStatusCode(drogon::k500InternalServerError);
                    callback(resp);
                },
                id
            );
        },
        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to delete patient appointments.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },
        id
    );
}
