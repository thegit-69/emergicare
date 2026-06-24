cmake_minimum_required(VERSION 3.16)
project(emergicare_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ============================================================
# External Dependencies via FetchContent
# ============================================================
include(FetchContent)

# jwt-cpp: header-only JWT library (C++ Capstone Requirement)
FetchContent_Declare(
    jwt-cpp
    GIT_REPOSITORY https://github.com/Thalhammer/jwt-cpp.git
    GIT_TAG        v0.7.0
)
set(JWT_BUILD_EXAMPLES OFF CACHE BOOL "Disable jwt-cpp examples" FORCE)
set(JWT_BUILD_TESTS    OFF CACHE BOOL "Disable jwt-cpp tests"    FORCE)
FetchContent_MakeAvailable(jwt-cpp)

# ============================================================
# Find System Packages
# ============================================================
find_package(Drogon CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# ============================================================
# Source Files
# ============================================================
set(SOURCES
    main.cpp
    src/utils/JwtHelper.cc
    src/utils/PasswordHelper.cc
    src/services/TriageService.cc
    src/middleware/AuthFilter.cc
    src/controllers/AuthController.cc
    src/controllers/UserController.cc
    src/controllers/PatientController.cc
    src/controllers/AppointmentController.cc
    src/controllers/MedicalRecordController.cc
    src/controllers/TriageController.cc
    src/controllers/EmergencyQueueController.cc
    src/controllers/ResourceController.cc
)

# ============================================================
# Build Target
# ============================================================
add_executable(${PROJECT_NAME} ${SOURCES})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Drogon::Drogon
    jwt-cpp::jwt-cpp
    OpenSSL::SSL
    OpenSSL::Crypto
    crypt           # libxcrypt: bcrypt support via crypt_r()
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
)

# Optional: Print build info
message(STATUS "----------------------------------------------")
message(STATUS "  EmergiCare C++ Backend (Drogon Framework)")
message(STATUS "  C++ Standard : C++${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type   : ${CMAKE_BUILD_TYPE}")
message(STATUS "----------------------------------------------")
