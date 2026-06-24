// models/MedicalRecord.h – Patient diagnosis / prescription record
#pragma once
#include <string>

struct MedicalRecord {
    int         id{0};
    int         patientId{0};
    std::string doctorName;
    std::string date;
    std::string diagnosis;
    std::string prescription;
    std::string notes;
    std::string createdAt;
};
