/**
 * TriageController.cc
 * ================================================================
 * This controller is the primary demonstration of C++ function overloading.
 *
 * When creating a triage entry (POST /api/triage), the controller
 * selects one of four TriageService::assess() overloads based on
 * which fields are present in the request body:
 *
 *   severityOverride present → Overload 4 (string, bool)
 *   vitals + complaint      → Overload 3 (Json::Value)     [default]
 *   vitals only             → Overload 2 (int, int, int)
 *   complaint only          → Overload 1 (string)
 * ================================================================
 */

#include "controllers/TriageController.h"
#include "services/TriageService.h"

#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

// Helper: build JSON object from a triage DB row
static Json::Value rowToTriage(const drogon::orm::Row& row) {
    Json::Value t;
    t["id"]             = row["id"].as<int>();
    t["patientId"]      = row["patient_id"].as<int>();
    t["patientName"]    = row["patient_name"].isNull() ? Json::Value("") : Json::Value(row["patient_name"].as<std::string>());
    t["severityLevel"]  = row["severity_level"].as<std::string>();
    t["severityLabel"]  = row["severity_label"].as<std::string>();
    t["chiefComplaint"] = row["chief_complaint"].isNull() ? Json::Value("") : Json::Value(row["chief_complaint"].as<std::string>());
    t["heartRate"]      = row["heart_rate"].isNull()    ? Json::Value(0) : Json::Value(row["heart_rate"].as<int>());
    t["systolicBp"]     = row["systolic_bp"].isNull()   ? Json::Value(0) : Json::Value(row["systolic_bp"].as<int>());
    t["diastolicBp"]    = row["diastolic_bp"].isNull()  ? Json::Value(0) : Json::Value(row["diastolic_bp"].as<int>());
    t["temperature"]    = row["temperature"].isNull()   ? Json::Value(0.0) : Json::Value(row["temperature"].as<double>());
    t["oxygenSat"]      = row["oxygen_sat"].isNull()    ? Json::Value(0) : Json::Value(row["oxygen_sat"].as<int>());
    t["assessedBy"]     = row["assessed_by"].as<std::string>();
    t["notes"]          = row["notes"].isNull() ? Json::Value("") : Json::Value(row["notes"].as<std::string>());
    t["status"]         = row["status"].as<std::string>();
    t["assessedAt"]     = row["assessed_at"].as<std::string>();
    return t;
}

// ---------------------------------------------------------------------------
// GET /api/triage – All entries, most critical first
// ---------------------------------------------------------------------------
void TriageController::getAll(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT te.id, te.patient_id, p.name AS patient_name, "
        "       te.severity_level, te.severity_label, te.chief_complaint, "
        "       te.heart_rate, te.systolic_bp, te.diastolic_bp, "
        "       te.temperature, te.oxygen_sat, te.assessed_by, "
        "       te.notes, te.status, te.assessed_at::text "
        "FROM triage_entries te "
        "LEFT JOIN patients p ON te.patient_id = p.id "
        "ORDER BY "
        "  CASE te.severity_level WHEN 'P1' THEN 1 WHEN 'P2' THEN 2 "
        "                         WHEN 'P3' THEN 3 ELSE 4 END, "
        "  te.assessed_at DESC",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToTriage(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch triage entries.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

// ---------------------------------------------------------------------------
// POST /api/triage – Assess patient and create triage entry
//
// This is where C++ function overloading is demonstrated:
// The controller inspects the request body and selects the appropriate
// TriageService::assess() overload at runtime based on available data.
// ---------------------------------------------------------------------------
void TriageController::create(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();
    if (!body || !body->isMember("patientId")) {
        Json::Value err; err["message"] = "patientId is required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    // ================================================================
    // SELECT THE CORRECT assess() OVERLOAD BASED ON REQUEST DATA
    // ================================================================
    TriagePriority priority;

    const bool hasOverride   = body->isMember("severityOverride") && !(*body)["severityOverride"].asString().empty();
    const bool hasVitals     = body->isMember("heartRate")  && body->isMember("systolicBp") &&
                               body->isMember("diastolicBp") &&
                               (*body)["heartRate"].asInt() > 0;
    const bool hasComplaint  = body->isMember("chiefComplaint") && !(*body)["chiefComplaint"].asString().empty();

    if (hasOverride) {
        // Overload 4: assess(string severity, bool isManualOverride)
        // Clinician manually sets the priority level
        priority = TriageService::assess((*body)["severityOverride"].asString(), true);

    } else if (hasVitals && hasComplaint) {
        // Overload 3: assess(const Json::Value& patientData)
        // Combined assessment using both symptoms and vital signs
        priority = TriageService::assess(*body);

    } else if (hasVitals) {
        // Overload 2: assess(int heartRate, int systolicBP, int diastolicBP)
        // Vital signs only — pure numeric threshold analysis
        priority = TriageService::assess(
            (*body)["heartRate"].asInt(),
            (*body)["systolicBp"].asInt(),
            (*body)["diastolicBp"].asInt()
        );

    } else {
        // Overload 1: assess(const std::string& chiefComplaint)
        // Keyword-based symptom analysis (fallback)
        const std::string complaint = hasComplaint ? (*body)["chiefComplaint"].asString() : "";
        priority = TriageService::assess(complaint);
    }

    // Retrieve the doctor's name from JWT claims (injected by AuthFilter)
    std::string assessedBy = "Unknown";
    try {
        assessedBy = req->attributes()->get<std::string>("name");
    } catch (...) {}

    const std::string severityCode  = priorityToCode(priority);
    const std::string severityLabel = priorityToLabel(priority);

    // Store in DB
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "INSERT INTO triage_entries "
        "(patient_id, severity_level, severity_label, chief_complaint, "
        " heart_rate, systolic_bp, diastolic_bp, temperature, oxygen_sat, "
        " assessed_by, notes, status) "
        "VALUES ($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, 'Waiting') "
        "RETURNING id, patient_id, severity_level, severity_label, chief_complaint, "
        "          heart_rate, systolic_bp, diastolic_bp, temperature, oxygen_sat, "
        "          assessed_by, notes, status, assessed_at::text",

        [callback](const drogon::orm::Result& r) {
            // For patient_name we do a second query in a real app; here we return what we have
            auto row = r[0];
            Json::Value t = rowToTriage(row);
            auto resp = drogon::HttpResponse::newHttpJsonResponse(t);
            resp->setStatusCode(drogon::k201Created);
            callback(resp);
        },

        [callback](const drogon::orm::DrogonDbException& e) {
            Json::Value err; err["message"] = std::string("Failed to create triage entry: ") + e.base().what();
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        (*body)["patientId"].asInt(),
        severityCode,
        severityLabel,
        hasComplaint ? (*body)["chiefComplaint"].asString() : "",
        hasVitals ? (*body)["heartRate"].asInt()   : 0,
        hasVitals ? (*body)["systolicBp"].asInt()  : 0,
        hasVitals ? (*body)["diastolicBp"].asInt() : 0,
        body->isMember("temperature") ? (*body)["temperature"].asDouble() : 0.0,
        body->isMember("oxygenSat")   ? (*body)["oxygenSat"].asInt()   : 0,
        assessedBy,
        (*body).get("notes", "").asString()
    );
}

// ---------------------------------------------------------------------------
// GET /api/triage/:id
// ---------------------------------------------------------------------------
void TriageController::getById(const drogon::HttpRequestPtr& req,
                                std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                int id) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT te.id, te.patient_id, p.name AS patient_name, "
        "       te.severity_level, te.severity_label, te.chief_complaint, "
        "       te.heart_rate, te.systolic_bp, te.diastolic_bp, "
        "       te.temperature, te.oxygen_sat, te.assessed_by, "
        "       te.notes, te.status, te.assessed_at::text "
        "FROM triage_entries te "
        "LEFT JOIN patients p ON te.patient_id = p.id "
        "WHERE te.id = $1",

        [callback](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err; err["message"] = "Triage entry not found.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(rowToTriage(r[0])));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch triage entry.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        id
    );
}

// ---------------------------------------------------------------------------
// PUT /api/triage/:id – Update status
// ---------------------------------------------------------------------------
void TriageController::update(const drogon::HttpRequestPtr& req,
                               std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                               int id) {
    auto body = req->getJsonObject();
    if (!body || !body->isMember("status")) {
        Json::Value err; err["message"] = "status field is required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "UPDATE triage_entries SET status = $1 WHERE id = $2 "
        "RETURNING id, patient_id, '' AS patient_name, severity_level, severity_label, "
        "          chief_complaint, heart_rate, systolic_bp, diastolic_bp, "
        "          temperature, oxygen_sat, assessed_by, notes, status, assessed_at::text",

        [callback](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err; err["message"] = "Triage entry not found.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(rowToTriage(r[0])));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to update triage entry.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        (*body)["status"].asString(),
        id
    );
}
