/**
 * ResourceController.cc – Hospital resource management
 * Tracks beds, ventilators, operating rooms, and staff availability.
 */

#include "controllers/ResourceController.h"
#include <drogon/drogon.h>
#include <json/json.h>

static Json::Value rowToResource(const drogon::orm::Row& row) {
    Json::Value res;
    res["id"]                 = row["id"].as<int>();
    res["resourceType"]       = row["resource_type"].as<std::string>();
    res["resourceName"]       = row["resource_name"].as<std::string>();
    res["location"]           = row["location"].isNull()       ? Json::Value("") : Json::Value(row["location"].as<std::string>());
    res["status"]             = row["status"].as<std::string>();
    res["assignedPatientId"]  = row["assigned_patient_id"].isNull() ? Json::Value(0) : Json::Value(row["assigned_patient_id"].as<int>());
    res["assignedPatientName"]= row["assigned_patient_name"].isNull() ? Json::Value("") : Json::Value(row["assigned_patient_name"].as<std::string>());
    res["assignedAt"]         = row["assigned_at"].isNull()    ? Json::Value("") : Json::Value(row["assigned_at"].as<std::string>());
    res["notes"]              = row["notes"].isNull()          ? Json::Value("") : Json::Value(row["notes"].as<std::string>());
    res["createdAt"]          = row["created_at"].as<std::string>();
    return res;
}

static const char* RESOURCE_SELECT =
    "SELECT r.id, r.resource_type, r.resource_name, r.location, r.status, "
    "       r.assigned_patient_id, p.name AS assigned_patient_name, "
    "       r.assigned_at::text, r.notes, r.created_at::text "
    "FROM resources r "
    "LEFT JOIN patients p ON r.assigned_patient_id = p.id ";

// ---------------------------------------------------------------------------
// GET /api/resources
// ---------------------------------------------------------------------------
void ResourceController::getAll(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        std::string(RESOURCE_SELECT) + "ORDER BY r.resource_type, r.resource_name",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToResource(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch resources.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

// ---------------------------------------------------------------------------
// GET /api/resources/available
// ---------------------------------------------------------------------------
void ResourceController::getAvailable(const drogon::HttpRequestPtr& req,
                                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        std::string(RESOURCE_SELECT) + "WHERE r.status = 'Available' ORDER BY r.resource_type",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToResource(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch available resources.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

// ---------------------------------------------------------------------------
// POST /api/resources – Add a new resource
// ---------------------------------------------------------------------------
void ResourceController::create(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();
    if (!body || !body->isMember("resourceType") || !body->isMember("resourceName")) {
        Json::Value err; err["message"] = "resourceType and resourceName are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "INSERT INTO resources (resource_type, resource_name, location, notes) "
        "VALUES ($1, $2, $3, $4) "
        "RETURNING id, resource_type, resource_name, location, status, "
        "          NULL::int AS assigned_patient_id, NULL::text AS assigned_patient_name, "
        "          NULL::text AS assigned_at, notes, created_at::text",

        [callback](const drogon::orm::Result& r) {
            auto resp = drogon::HttpResponse::newHttpJsonResponse(rowToResource(r[0]));
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to create resource.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        (*body)["resourceType"].asString(),
        (*body)["resourceName"].asString(),
        (*body).get("location", "").asString(),
        (*body).get("notes", "").asString()
    );
}

// ---------------------------------------------------------------------------
// PUT /api/resources/:id – Allocate or release a resource
// Body: { status: "In Use"|"Available"|"Maintenance", assignedPatientId?: int }
// ---------------------------------------------------------------------------
void ResourceController::update(const drogon::HttpRequestPtr& req,
                                 std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                 int id) {
    auto body = req->getJsonObject();
    if (!body || !body->isMember("status")) {
        Json::Value err; err["message"] = "status is required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    const std::string status = (*body)["status"].asString();
    const bool        hasPatient = body->isMember("assignedPatientId") &&
                                   !(*body)["assignedPatientId"].isNull() &&
                                   (*body)["assignedPatientId"].asInt() > 0;

    auto db = drogon::app().getDbClient();

    if (status == "Available") {
        // Release: clear patient assignment
        db->execSqlAsync(
            "UPDATE resources SET status = 'Available', "
            "       assigned_patient_id = NULL, assigned_at = NULL "
            "WHERE id = $1 "
            "RETURNING id, resource_type, resource_name, location, status, "
            "          NULL::int AS assigned_patient_id, NULL::text AS assigned_patient_name, "
            "          NULL::text AS assigned_at, notes, created_at::text",

            [callback](const drogon::orm::Result& r) {
                if (r.empty()) {
                    Json::Value err; err["message"] = "Resource not found.";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                    resp->setStatusCode(drogon::k404NotFound);
                    callback(resp); return;
                }
                callback(drogon::HttpResponse::newHttpJsonResponse(rowToResource(r[0])));
            },

            [callback](const drogon::orm::DrogonDbException&) {
                Json::Value err; err["message"] = "Failed to release resource.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                callback(resp);
            },
            id
        );
    } else {
        // Allocate or set Maintenance
        const int patientId = hasPatient ? (*body)["assignedPatientId"].asInt() : 0;
        db->execSqlAsync(
            "UPDATE resources SET status = $1, "
            "       assigned_patient_id = NULLIF($2, 0), "
            "       assigned_at = CASE WHEN $2 > 0 THEN NOW() ELSE assigned_at END "
            "WHERE id = $3 "
            "RETURNING id, resource_type, resource_name, location, status, "
            "          assigned_patient_id, NULL::text AS assigned_patient_name, "
            "          assigned_at::text, notes, created_at::text",

            [callback](const drogon::orm::Result& r) {
                if (r.empty()) {
                    Json::Value err; err["message"] = "Resource not found.";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                    resp->setStatusCode(drogon::k404NotFound);
                    callback(resp); return;
                }
                callback(drogon::HttpResponse::newHttpJsonResponse(rowToResource(r[0])));
            },

            [callback](const drogon::orm::DrogonDbException&) {
                Json::Value err; err["message"] = "Failed to update resource.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k500InternalServerError);
                callback(resp);
            },

            status, patientId, id
        );
    }
}
