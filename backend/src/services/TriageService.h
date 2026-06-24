/**
 * TriageService.h
 * ================================================================
 * C++ CAPSTONE REQUIREMENT: FUNCTION OVERLOADING
 * ================================================================
 *
 * This service demonstrates function overloading — a core C++ feature
 * where multiple functions share the same name but differ in their
 * parameter types and/or count.
 *
 * The assess() method is overloaded with 4 different signatures:
 *
 *   Overload 1: assess(string)            – keyword-based symptom analysis
 *   Overload 2: assess(int, int, int)     – vital signs threshold logic
 *   Overload 3: assess(Json::Value)       – combined JSON record assessment
 *   Overload 4: assess(string, bool)      – manual clinician override
 *
 * The compiler selects the correct overload at compile time based on
 * the argument types provided by the caller (TriageController).
 */

#pragma once

#include <string>
#include <json/json.h>

// ---------------------------------------------------------------------------
// Triage priority levels (P1 = most critical → P4 = least critical)
// ---------------------------------------------------------------------------
enum class TriagePriority : int {
    IMMEDIATE   = 1,   // P1 – Life-threatening, requires immediate intervention
    URGENT      = 2,   // P2 – Potentially life-threatening, treat within 10 min
    LESS_URGENT = 3,   // P3 – Stable, treat within 30–60 min
    NON_URGENT  = 4    // P4 – Minor, can safely wait
};

/** Convert TriagePriority enum → short code string ("P1", "P2", ...) */
std::string priorityToCode(TriagePriority priority);

/** Convert TriagePriority enum → human-readable label */
std::string priorityToLabel(TriagePriority priority);

// ---------------------------------------------------------------------------
// TriageService
// ---------------------------------------------------------------------------
class TriageService {
public:
    // ================================================================
    // OVERLOADED assess() FUNCTIONS  (C++ Capstone Requirement)
    // ================================================================

    /**
     * Overload 1 – Symptom keyword analysis
     *
     * Scans the chief complaint text for known emergency keywords and
     * classifies severity accordingly. Uses std::string as parameter.
     *
     * @param chiefComplaint  Free-text description of the patient's complaint
     * @return  TriagePriority determined by keyword matching
     */
    static TriagePriority assess(const std::string& chiefComplaint);

    /**
     * Overload 2 – Vital signs threshold logic
     *
     * Applies clinical threshold rules to numeric vital sign measurements.
     * Parameter types (int, int, int) distinguish this from Overload 1.
     *
     * @param heartRate     Beats per minute
     * @param systolicBP    Systolic blood pressure (mmHg)
     * @param diastolicBP   Diastolic blood pressure (mmHg)
     * @return  TriagePriority based on vital sign thresholds
     */
    static TriagePriority assess(int heartRate, int systolicBP, int diastolicBP);

    /**
     * Overload 3 – Full JSON patient record (combined assessment)
     *
     * Combines Overload 1 (symptoms) and Overload 2 (vitals) and returns
     * the highest severity level found. The Json::Value parameter type
     * makes this overload unique at compile time.
     *
     * @param patientData  JSON object with optional fields:
     *                     chiefComplaint, heartRate, systolicBp, diastolicBp
     * @return  TriagePriority – worst-case of symptom + vitals assessment
     */
    static TriagePriority assess(const Json::Value& patientData);

    /**
     * Overload 4 – Manual clinician override
     *
     * Allows a doctor to manually set the triage level, bypassing
     * automated assessment. The bool parameter distinguishes this
     * overload from Overload 1 (same first parameter type).
     *
     * @param severity          "P1" | "P2" | "P3" | "P4" or level label
     * @param isManualOverride  Must be true to trigger direct mapping;
     *                          if false, falls back to Overload 1
     * @return  TriagePriority from the provided severity string
     */
    static TriagePriority assess(const std::string& severity, bool isManualOverride);

    // ================================================================
    // Helpers
    // ================================================================

    /** Convert a priority code string ("P1"–"P4") to enum */
    static TriagePriority fromCode(const std::string& code);

    /** Compute the integer priority_score stored in the queue (1=highest) */
    static int toPriorityScore(TriagePriority p) {
        return static_cast<int>(p);
    }
};
