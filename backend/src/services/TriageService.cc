/**
 * TriageService.cc
 * ================================================================
 * C++ CAPSTONE REQUIREMENT: FUNCTION OVERLOADING IMPLEMENTATION
 * ================================================================
 *
 * Four overloaded assess() methods are implemented here.
 * Each accepts a distinct set of parameter types, demonstrating
 * how C++ resolves function calls based on argument types at
 * compile time — a fundamental feature of the C++ type system.
 */

#include "services/TriageService.h"

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Helper conversions
// ---------------------------------------------------------------------------

std::string priorityToCode(TriagePriority priority) {
    switch (priority) {
        case TriagePriority::IMMEDIATE:   return "P1";
        case TriagePriority::URGENT:      return "P2";
        case TriagePriority::LESS_URGENT: return "P3";
        case TriagePriority::NON_URGENT:  return "P4";
        default:                           return "P4";
    }
}

std::string priorityToLabel(TriagePriority priority) {
    switch (priority) {
        case TriagePriority::IMMEDIATE:   return "Immediate";
        case TriagePriority::URGENT:      return "Urgent";
        case TriagePriority::LESS_URGENT: return "Less Urgent";
        case TriagePriority::NON_URGENT:  return "Non-Urgent";
        default:                           return "Non-Urgent";
    }
}

TriagePriority TriageService::fromCode(const std::string& code) {
    if (code == "P1" || code == "IMMEDIATE")    return TriagePriority::IMMEDIATE;
    if (code == "P2" || code == "URGENT")       return TriagePriority::URGENT;
    if (code == "P3" || code == "LESS_URGENT")  return TriagePriority::LESS_URGENT;
    return TriagePriority::NON_URGENT;
}

// ---------------------------------------------------------------------------
// Internal: lowercase a string (for case-insensitive keyword matching)
// ---------------------------------------------------------------------------
static std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// ============================================================================
//  OVERLOAD 1 – const std::string& chiefComplaint
//  Keyword-based symptom classification
// ============================================================================
TriagePriority TriageService::assess(const std::string& chiefComplaint) {
    const std::string lower = toLower(chiefComplaint);

    // P1 – IMMEDIATE: life-threatening keywords
    // These symptoms require resuscitation / immediate physician attention
    static const std::vector<std::string> p1Keywords = {
        "cardiac arrest", "heart attack", "stroke", "not breathing",
        "unconscious", "unresponsive", "severe bleeding", "anaphylaxis",
        "anaphylactic shock", "seizure", "gunshot", "stab wound",
        "choking", "drowning", "overdose", "respiratory arrest",
        "no pulse", "major trauma"
    };
    for (const auto& kw : p1Keywords) {
        if (lower.find(kw) != std::string::npos) {
            return TriagePriority::IMMEDIATE;
        }
    }

    // P2 – URGENT: potentially life-threatening, treat within 10 minutes
    static const std::vector<std::string> p2Keywords = {
        "chest pain", "difficulty breathing", "shortness of breath",
        "severe pain", "head injury", "broken bone", "fracture",
        "high fever", "abdominal pain", "severe allergic", "allergic reaction",
        "confusion", "altered consciousness", "severe headache",
        "vomiting blood", "diabetic emergency", "severe burn",
        "suspected poisoning", "eye injury", "suicidal"
    };
    for (const auto& kw : p2Keywords) {
        if (lower.find(kw) != std::string::npos) {
            return TriagePriority::URGENT;
        }
    }

    // P3 – LESS URGENT: stable but needs timely attention
    static const std::vector<std::string> p3Keywords = {
        "moderate pain", "fever", "nausea", "vomiting", "dizziness",
        "urinary infection", "sprain", "laceration", "asthma",
        "ear pain", "back pain", "rash", "migraine", "anxiety",
        "mild burn", "wound", "swelling", "constipation"
    };
    for (const auto& kw : p3Keywords) {
        if (lower.find(kw) != std::string::npos) {
            return TriagePriority::LESS_URGENT;
        }
    }

    // P4 – NON-URGENT: minor complaints, can safely wait
    return TriagePriority::NON_URGENT;
}

// ============================================================================
//  OVERLOAD 2 – int heartRate, int systolicBP, int diastolicBP
//  Vital signs numeric threshold classification
// ============================================================================
TriagePriority TriageService::assess(int heartRate, int systolicBP, int diastolicBP) {
    // P1 – IMMEDIATE: critically abnormal vitals
    // Source: ACLS / AHA emergency thresholds
    if (heartRate    <  40 || heartRate   > 150 ||
        systolicBP   <  70 || systolicBP  > 220 ||
        diastolicBP  > 130) {
        return TriagePriority::IMMEDIATE;
    }

    // P2 – URGENT: significantly abnormal vitals
    if (heartRate    <  50 || heartRate   > 130 ||
        systolicBP   <  90 || systolicBP  > 180 ||
        diastolicBP  > 110) {
        return TriagePriority::URGENT;
    }

    // P3 – LESS URGENT: mildly abnormal vitals
    if (heartRate    <  60 || heartRate   > 100 ||
        systolicBP   < 100 || systolicBP  > 140 ||
        diastolicBP  >  90) {
        return TriagePriority::LESS_URGENT;
    }

    // P4 – NON-URGENT: vitals within normal range
    return TriagePriority::NON_URGENT;
}

// ============================================================================
//  OVERLOAD 3 – const Json::Value& patientData
//  Combined assessment from full JSON patient record
// ============================================================================
TriagePriority TriageService::assess(const Json::Value& patientData) {
    TriagePriority symptomPriority = TriagePriority::NON_URGENT;
    TriagePriority vitalsPriority  = TriagePriority::NON_URGENT;

    // --- Call Overload 1 if chiefComplaint is present ---
    if (patientData.isMember("chiefComplaint") &&
        !patientData["chiefComplaint"].isNull() &&
        !patientData["chiefComplaint"].asString().empty()) {

        // Compile-time dispatch → Overload 1 (std::string)
        symptomPriority = assess(patientData["chiefComplaint"].asString());
    }

    // --- Call Overload 2 if vital signs are present and non-zero ---
    if (patientData.isMember("heartRate")   && !patientData["heartRate"].isNull()  &&
        patientData.isMember("systolicBp")  && !patientData["systolicBp"].isNull() &&
        patientData.isMember("diastolicBp") && !patientData["diastolicBp"].isNull() &&
        patientData["heartRate"].asInt()  > 0 &&
        patientData["systolicBp"].asInt() > 0) {

        // Compile-time dispatch → Overload 2 (int, int, int)
        vitalsPriority = assess(
            patientData["heartRate"].asInt(),
            patientData["systolicBp"].asInt(),
            patientData["diastolicBp"].asInt()
        );
    }

    // Return the higher severity (lower numeric value = more critical)
    return (static_cast<int>(symptomPriority) <= static_cast<int>(vitalsPriority))
           ? symptomPriority
           : vitalsPriority;
}

// ============================================================================
//  OVERLOAD 4 – const std::string& severity, bool isManualOverride
//  Manual clinician priority override
// ============================================================================
TriagePriority TriageService::assess(const std::string& severity, bool isManualOverride) {
    if (isManualOverride) {
        // Direct string-to-enum mapping; bypass keyword analysis entirely
        // Compile-time dispatch: fromCode() is not another assess() overload,
        // so there's no recursion here.
        return fromCode(severity);
    }

    // If not a manual override, fall back to keyword-based analysis
    // Compile-time dispatch → Overload 1 (std::string)
    return assess(severity);
}
