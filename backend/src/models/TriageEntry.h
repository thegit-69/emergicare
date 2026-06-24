// models/TriageEntry.h – Emergency triage assessment record
#pragma once
#include <string>

struct TriageEntry {
    int         id{0};
    int         patientId{0};
    std::string patientName;
    std::string severityLevel;  // "P1" | "P2" | "P3" | "P4"
    std::string severityLabel;  // "Immediate" | "Urgent" | "Less Urgent" | "Non-Urgent"
    std::string chiefComplaint;
    int         heartRate{0};
    int         systolicBp{0};
    int         diastolicBp{0};
    double      temperature{0.0};
    int         oxygenSat{0};
    std::string assessedBy;
    std::string notes;
    std::string status;         // "Waiting" | "In Treatment" | "Discharged"
    std::string assessedAt;
};
