// models/EmergencyQueueEntry.h – Entry in the live emergency queue
#pragma once
#include <string>

struct EmergencyQueueEntry {
    int         id{0};
    int         triageId{0};
    int         patientId{0};
    std::string patientName;
    std::string severityLevel;          // "P1" | "P2" | "P3" | "P4"
    int         priorityScore{0};       // 1 = highest, computed from severity
    std::string queueStatus;            // "Waiting" | "In Treatment" | "Discharged"
    std::string assignedDoctor;
    std::string assignedBed;
    std::string enqueuedAt;
    std::string treatmentStartedAt;
    std::string dischargedAt;
};
