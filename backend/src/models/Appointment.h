// models/Appointment.h – Scheduled patient appointment
#pragma once
#include <string>

struct Appointment {
    int         id{0};
    int         patientId{0};
    std::string patientName;
    std::string doctorName;
    std::string date;       // ISO datetime
    std::string reason;
    std::string status;     // "Scheduled" | "Completed" | "Cancelled"
    std::string createdAt;
};
