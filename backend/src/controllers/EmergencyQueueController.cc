/**
 * EmergencyQueueController.cc
 * Live emergency queue management.
 * Queue is ordered: P1 first → P4 last, FIFO within same priority.
 */

#include "controllers/EmergencyQueueController.h"
#include <drogon/drogon.h>
#include <json/json.h>
#include <string>

static Json::Value rowToQueueEntry(const drogon::orm::Row& row) {
    Json::Value e;
    try {
        e["id"]                 = row["id"].as<int>();
        e["triageId"]           = row["triage_id"].as<int>();
        e["patientId"]          = row["patient_id"].as<int>();
        e["patientName"]        = row["patient_name"].isNull() ? Json::Value("") : Json::Value(row["patient_name"].as<std::string>());
        e["severityLevel"]      = row["severity_level"].isNull() ? Json::Value("") : Json::Value(row["severity_level"].as<std::string>());
        e["severityLabel"]      = row["severity_label"].isNull() ? Json::Value("") : Json::Value(row["severity_label"].as<std::string>());
        e["priorityScore"]      = row["priority_score"].as<int>();
        e["queueStatus"]        = row["queue_status"].as<std::string>();
        e["assignedDoctor"]     = row["assigned_doctor"].isNull() ? Json::Value("") : Json::Value(row["assigned_doctor"].as<std::string>());
        e["assignedBed"]        = row["assigned_bed"].isNull()    ? Json::Value("") : Json::Value(row["assigned_bed"].as<std::string>());
        e["enqueuedAt"]         = row["enqueued_at"].as<std::string>();
        e["treatmentStartedAt"] = row["treatment_started_at"].isNull() ? Json::Value("") : Json::Value(row["treatment_started_at"].as<std::string>());
        e["dischargedAt"]       = row["discharged_at"].isNull()        ? Json::Value("") : Json::Value(row["discharged_at"].as<std::string>());
    } catch (...) {
        // Fallback for fields
    }
    return e;
}

// ---------------------------------------------------------------------------
// GET /api/emergency-queue
// ---------------------------------------------------------------------------
void EmergencyQueueController::getAll(const drogon::HttpRequestPtr& req,
                                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT eq.id, eq.triage_id, eq.patient_id, p.name AS patient_name, "
        "       te.severity_level, te.severity_label, "
        "       eq.priority_score, eq.queue_status, "
        "       eq.assigned_doctor, eq.assigned_bed, "
        "       eq.enqueued_at::text, eq.treatment_started_at::text, "
        "       eq.discharged_at::text "
        "FROM emergency_queue eq "
        "JOIN patients p        ON eq.patient_id = p.id "
        "JOIN triage_entries te ON eq.triage_id  = te.id "
        "ORDER BY eq.priority_score ASC, eq.enqueued_at ASC",

        [callback](const drogon::orm::Result& r) {
            Json::Value arr(Json::arrayValue);
            for (const auto& row : r) arr.append(rowToQueueEntry(row));
            callback(drogon::HttpResponse::newHttpJsonResponse(arr));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch emergency queue.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        }
    );
}

// ---------------------------------------------------------------------------
// POST /api/emergency-queue – Enqueue triaged patient
// Body: { triageId, patientId }
// ---------------------------------------------------------------------------
void EmergencyQueueController::enqueue(const drogon::HttpRequestPtr& req,
                                        std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
    auto body = req->getJsonObject();
    if (!body || !body->isMember("triageId") || !body->isMember("patientId")) {
        Json::Value err; err["message"] = "triageId and patientId are required.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    const int triageId  = (*body)["triageId"].asInt();
    const int patientId = (*body)["patientId"].asInt();

    // Fetch the severity level from the triage entry to compute priority_score
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "SELECT severity_level FROM triage_entries WHERE id = $1",

        [db, callback, triageId, patientId](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err; err["message"] = "Triage entry not found.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }

            const std::string severity = r[0]["severity_level"].as<std::string>();
            int priorityScore = 4; // default P4
            if (severity == "P1") priorityScore = 1;
            else if (severity == "P2") priorityScore = 2;
            else if (severity == "P3") priorityScore = 3;

            db->execSqlAsync(
                "WITH inserted AS ( "
                "  INSERT INTO emergency_queue "
                "  (triage_id, patient_id, priority_score, queue_status) "
                "  VALUES ($1, $2, $3, 'Waiting') "
                "  RETURNING * "
                ") "
                "SELECT eq.id, eq.triage_id, eq.patient_id, p.name AS patient_name, "
                "       te.severity_level, te.severity_label, "
                "       eq.priority_score, eq.queue_status, "
                "       eq.assigned_doctor, eq.assigned_bed, "
                "       eq.enqueued_at::text, eq.treatment_started_at::text, "
                "       eq.discharged_at::text "
                "FROM inserted eq "
                "JOIN patients p        ON eq.patient_id = p.id "
                "JOIN triage_entries te ON eq.triage_id  = te.id",

                [callback](const drogon::orm::Result& r2) {
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(rowToQueueEntry(r2[0]));
                    resp->setStatusCode(drogon::k201Created);
                    callback(resp);
                },

                [callback](const drogon::orm::DrogonDbException& e) {
                    Json::Value err; err["message"] = "Failed to enqueue patient.";
                    auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                    resp->setStatusCode(drogon::k500InternalServerError);
                    callback(resp);
                },

                triageId, patientId, priorityScore
            );
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to fetch triage entry for queueing.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        triageId
    );
}

// ---------------------------------------------------------------------------
// PUT /api/emergency-queue/:id – Update status / assign doctor / assign bed
// ---------------------------------------------------------------------------
void EmergencyQueueController::update(const drogon::HttpRequestPtr& req,
                                       std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                       int id) {
    auto body = req->getJsonObject();
    if (!body) {
        Json::Value err; err["message"] = "Invalid JSON body.";
        auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
        resp->setStatusCode(drogon::k400BadRequest);
        callback(resp); return;
    }

    const std::string status         = (*body).get("queueStatus",    "").asString();
    const std::string assignedDoctor = (*body).get("assignedDoctor", "").asString();
    const std::string assignedBed    = (*body).get("assignedBed",    "").asString();

    // Set treatment_started_at when status changes to "In Treatment"
    const std::string startedSql = (status == "In Treatment")
        ? ", treatment_started_at = NOW()" : "";

    // Set discharged_at when status changes to "Discharged"
    const std::string dischargedSql = (status == "Discharged")
        ? ", discharged_at = NOW()" : "";

    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "WITH old_state AS ( "
        "  SELECT assigned_bed FROM emergency_queue WHERE id = $4 "
        "), "
        "updated AS ( "
        "  UPDATE emergency_queue "
        "  SET queue_status    = COALESCE(NULLIF($1,''), queue_status), "
        "      assigned_doctor = COALESCE(NULLIF($2,''), assigned_doctor), "
        "      assigned_bed    = COALESCE(NULLIF($3,''), assigned_bed) "
        + startedSql + dischargedSql +
        "  WHERE id = $4 "
        "  RETURNING * "
        "), "
        "updated_triage AS ( "
        "  UPDATE triage_entries "
        "  SET status = COALESCE(NULLIF($1,''), status) "
        "  FROM updated "
        "  WHERE triage_entries.id = updated.triage_id "
        "  AND $1 IN ('In Treatment', 'Discharged') "
        "), "
        "assigned_resource AS ( "
        "  UPDATE resources "
        "  SET status = 'In Use', "
        "      assigned_patient_id = updated.patient_id, "
        "      assigned_at = NOW() "
        "  FROM updated "
        "  WHERE resources.resource_name = NULLIF($3,'') "
        "), "
        "released_resource AS ( "
        "  UPDATE resources "
        "  SET status = 'Available', "
        "      assigned_patient_id = NULL, "
        "      assigned_at = NULL "
        "  FROM old_state, updated "
        "  WHERE resources.resource_name = old_state.assigned_bed "
        "  AND old_state.assigned_bed IS NOT NULL "
        "  AND ( $1 = 'Discharged' OR (NULLIF($3,'') IS NOT NULL AND old_state.assigned_bed != $3) ) "
        ") "
        "SELECT eq.id, eq.triage_id, eq.patient_id, p.name AS patient_name, "
        "       te.severity_level, te.severity_label, "
        "       eq.priority_score, eq.queue_status, "
        "       eq.assigned_doctor, eq.assigned_bed, "
        "       eq.enqueued_at::text, eq.treatment_started_at::text, "
        "       eq.discharged_at::text "
        "FROM updated eq "
        "JOIN patients p        ON eq.patient_id = p.id "
        "JOIN triage_entries te ON eq.triage_id  = te.id",

        [callback](const drogon::orm::Result& r) {
            if (r.empty()) {
                Json::Value err; err["message"] = "Queue entry not found.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }
            callback(drogon::HttpResponse::newHttpJsonResponse(rowToQueueEntry(r[0])));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to update queue entry.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        status, assignedDoctor, assignedBed, id
    );
}

// ---------------------------------------------------------------------------
// DELETE /api/emergency-queue/:id – Remove from queue
// ---------------------------------------------------------------------------
void EmergencyQueueController::dequeue(const drogon::HttpRequestPtr& req,
                                        std::function<void(const drogon::HttpResponsePtr&)>&& callback,
                                        int id) {
    auto db = drogon::app().getDbClient();
    db->execSqlAsync(
        "DELETE FROM emergency_queue WHERE id = $1",

        [callback](const drogon::orm::Result& r) {
            if (r.affectedRows() == 0) {
                Json::Value err; err["message"] = "Queue entry not found.";
                auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
                resp->setStatusCode(drogon::k404NotFound);
                callback(resp); return;
            }
            Json::Value ok; ok["success"] = true; ok["message"] = "Removed from queue.";
            callback(drogon::HttpResponse::newHttpJsonResponse(ok));
        },

        [callback](const drogon::orm::DrogonDbException&) {
            Json::Value err; err["message"] = "Failed to remove queue entry.";
            auto resp = drogon::HttpResponse::newHttpJsonResponse(err);
            resp->setStatusCode(drogon::k500InternalServerError);
            callback(resp);
        },

        id
    );
}
