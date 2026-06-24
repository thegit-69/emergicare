// models/User.h – System staff account
#pragma once
#include <string>

struct User {
    int         id{0};
    std::string username;
    std::string role;       // "Admin" | "Doctor" | "Receptionist"
    std::string name;
    std::string createdAt;
};
