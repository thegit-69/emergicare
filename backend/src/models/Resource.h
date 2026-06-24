// models/Resource.h – Hospital resource (bed, ventilator, OR, staff)
#pragma once
#include <string>

struct Resource {
    int         id{0};
    std::string resourceType;   // "Bed" | "Ventilator" | "OperatingRoom" | "Staff"
    std::string resourceName;
    std::string location;
    std::string status;         // "Available" | "In Use" | "Maintenance"
    int         assignedPatientId{0};
    std::string assignedAt;
    std::string notes;
    std::string createdAt;
};
