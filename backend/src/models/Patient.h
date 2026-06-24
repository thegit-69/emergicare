// models/Patient.h – Patient demographic record
#pragma once
#include <string>

struct Patient {
    int         id{0};
    std::string name;
    std::string dob;        // ISO date string "YYYY-MM-DD"
    std::string gender;     // "Male" | "Female" | "Other"
    std::string contact;
    std::string email;
    std::string address;
    std::string createdAt;
};
