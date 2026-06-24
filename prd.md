# EmergiCare — Product Requirements Document (PRD)
**An Adaptive Emergency Care Coordination System for Streamlining Clinical Workflows Through Function Overloading**
**Course:** C++ Capstone Project
**Version:** 2.0.0
**Date:** June 2026

---

## 1. Project Overview

### 1.1 Background
The existing project ("Mediflow") is a hospital management system with a Node.js/Express backend, a React/Vite frontend, and a PostgreSQL database. It supports three roles (Admin, Doctor, Receptionist) and manages patients, appointments, and medical records.

### 1.2 Goal
Migrate the backend to **C++ using the Drogon web framework** running inside **Docker containers**, while:
- Keeping the existing React frontend **functionally and visually intact**
- Adding three new emergency-focused modules: **Triage**, **Emergency Queue**, and **Resource Allocation**
- Demonstrating **C++ function overloading** as the primary academic concept
- Updating the branding to **EmergiCare**

---

## 2. Technology Stack

| Layer       | Old (v1)            | New (v2)                          |
|-------------|---------------------|-----------------------------------|
| Backend     | Node.js + Express   | **C++ 17 + Drogon Framework**     |
| Auth        | jsonwebtoken (npm)  | **jwt-cpp (header-only)**         |
| Password    | bcryptjs (npm)      | **bcrypt (libsodium or OpenSSL)** |
| Database    | PostgreSQL via `pg` | **PostgreSQL via libpqxx**        |
| Frontend    | React + Vite        | React + Vite *(unchanged)*        |
| Styling     | Tailwind CSS v4     | Tailwind CSS v4 *(unchanged)*     |
| Container   | Docker (Node image) | **Docker (Ubuntu + CMake build)** |
| Orchestration | docker-compose   | docker-compose *(expanded)*       |

---

## 3. File Change Plan

### 3.1 Files to DELETE

| File/Directory              | Reason |
|-----------------------------|--------|
| `backend/` (entire)         | Replace Node.js backend with C++ |
| `COMPLETE_SETUP.md`         | Node.js-specific, obsolete |
| `MIGRATION_SUMMARY.md`      | Node.js-specific, obsolete |
| `POSTGRES_SETUP_GUIDE.md`   | Replaced by Docker Compose managed DB |
| `QUICK_REFERENCE.md`        | Obsolete |
| `package.json` (root)       | Root-level Node workspace no longer needed |
| `package-lock.json` (root)  | Same reason |
| `skills-lock.json` (root)   | Not relevant |

### 3.2 Files to KEEP

| File/Directory      | Changes Required |
|---------------------|------------------|
| `frontend/`         | Minor: rename branding, add 3 new views, add 3 nav items |
| `init-db.sql`       | Add 3 new emergency tables |
| `docker-compose.yml`| Rewrite: add postgres service, replace Node backend with C++ backend |
| `.gitignore`        | Add C++ build artifacts |
| `README.md`         | Rewrite with new setup instructions |

### 3.3 Files to CREATE

| File/Directory                          | Purpose |
|-----------------------------------------|---------|
| `backend/CMakeLists.txt`                | CMake build configuration |
| `backend/Dockerfile`                    | Multi-stage C++ Docker build |
| `backend/config.json`                   | Drogon server configuration |
| `backend/main.cpp`                      | Application entry point |
| `backend/src/controllers/*.h/.cc`       | REST API controllers (8 files) |
| `backend/src/middleware/AuthFilter.h/.cc` | JWT authentication middleware |
| `backend/src/models/*.h`               | Data model structs (7 files) |
| `backend/src/utils/JwtHelper.h/.cc`    | JWT utility |
| `backend/src/utils/PasswordHelper.h/.cc`| bcrypt password utility |
| `backend/src/services/TriageService.h/.cc` | Function overloading demo |

---

## 4. Feature Requirements

### 4.1 Existing Features (Preserved)

#### 4.1.1 Authentication & RBAC
- **POST /api/login** — Username + password → JWT token (7-day expiry)
- **POST /api/users** — Create new staff user (Admin only via frontend gate)
- **GET /api/users** — List all users (requires JWT)
- Roles: `Admin`, `Doctor`, `Receptionist`
- JWT must include: `id`, `username`, `role`, `name`

#### 4.1.2 Patient Management
- **GET /api/patients** — All patients (all roles)
- **POST /api/patients** — Register patient (Admin, Doctor, Receptionist)
- **PUT /api/patients/:id** — Update patient (Admin, Doctor)
- **DELETE /api/patients/:id** — Delete patient + cascade records (Admin only)

#### 4.1.3 Appointments
- **GET /api/appointments** — All appointments
- **POST /api/appointments** — Schedule new appointment

#### 4.1.4 Medical Records
- **GET /api/medical-records** — All records
- **POST /api/medical-records** — Add new record (Doctor only via frontend gate)

### 4.2 New Features

#### 4.2.1 Triage System
**Endpoints:**
- `GET /api/triage` — All triage entries, ordered by severity
- `POST /api/triage` — Assess and register a patient for triage
- `GET /api/triage/:id` — Get specific triage entry
- `PUT /api/triage/:id` — Update triage status

**Triage Severity Levels:**

| Level | Code | Description              | Color (UI) |
|-------|------|--------------------------|------------|
| P1    | IMMEDIATE    | Life-threatening        | Red        |
| P2    | URGENT       | Potentially life-threatening | Orange |
| P3    | LESS_URGENT  | Stable, needs care      | Yellow     |
| P4    | NON_URGENT   | Minor, can wait         | Green      |

**C++ Function Overloading (Core Academic Requirement):**

```cpp
// In TriageService.h
class TriageService {
public:
    // Overload 1: Classify by chief complaint (keyword matching)
    static TriagePriority assess(const std::string& chiefComplaint);

    // Overload 2: Classify by vital signs (threshold logic)
    static TriagePriority assess(int heartRate, int systolicBP, int diastolicBP);

    // Overload 3: Classify from full JSON input (combined assessment)
    static TriagePriority assess(const Json::Value& patientData);

    // Overload 4: Accept manual override (string + boolean flag)
    static TriagePriority assess(const std::string& severity, bool isManualOverride);
};
```

The controller calls the appropriate overload based on what data the frontend provides.

#### 4.2.2 Emergency Queue
**Endpoints:**
- `GET /api/emergency-queue` — Ordered queue (P1 first, then by arrival time)
- `POST /api/emergency-queue` — Enqueue patient after triage
- `PUT /api/emergency-queue/:id` — Update status (Waiting → In Treatment → Discharged)
- `DELETE /api/emergency-queue/:id` — Remove from queue

**Queue Ordering Logic:**
1. Primary sort: `severity_level` (P1 → P2 → P3 → P4)
2. Secondary sort: `enqueued_at` (FIFO within same priority)

#### 4.2.3 Resource Allocation
**Endpoints:**
- `GET /api/resources` — All resources with current status
- `POST /api/resources` — Add new resource
- `PUT /api/resources/:id` — Allocate/release resource
- `GET /api/resources/available` — Only available resources

**Resource Types:** Bed, Ventilator, OperatingRoom, Staff

---

## 5. Database Schema (Updated)

### 5.1 Existing Tables (Unchanged Structure)
- `users` — system staff accounts
- `patients` — patient demographic data
- `appointments` — scheduled appointments
- `medical_records` — diagnosis and prescription records

### 5.2 New Tables

```sql
-- Triage assessment records
CREATE TABLE triage_entries (
    id              SERIAL PRIMARY KEY,
    patient_id      INTEGER REFERENCES patients(id) ON DELETE CASCADE,
    severity_level  VARCHAR(20) NOT NULL CHECK (severity_level IN ('P1', 'P2', 'P3', 'P4')),
    chief_complaint TEXT,
    heart_rate      INTEGER,
    systolic_bp     INTEGER,
    diastolic_bp    INTEGER,
    temperature     DECIMAL(4,1),
    oxygen_sat      INTEGER,
    assessed_by     VARCHAR(100),
    notes           TEXT,
    status          VARCHAR(30) DEFAULT 'Waiting' CHECK (status IN ('Waiting', 'In Treatment', 'Discharged')),
    assessed_at     TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- Emergency queue management
CREATE TABLE emergency_queue (
    id                  SERIAL PRIMARY KEY,
    triage_id           INTEGER REFERENCES triage_entries(id) ON DELETE CASCADE,
    patient_id          INTEGER REFERENCES patients(id) ON DELETE CASCADE,
    priority_score      INTEGER NOT NULL,   -- 1=highest, computed from severity
    queue_status        VARCHAR(30) DEFAULT 'Waiting',
    assigned_doctor     VARCHAR(100),
    assigned_bed        VARCHAR(20),
    enqueued_at         TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    treatment_started_at TIMESTAMP,
    discharged_at       TIMESTAMP
);

-- Hospital resource tracking
CREATE TABLE resources (
    id                  SERIAL PRIMARY KEY,
    resource_type       VARCHAR(50) NOT NULL,
    resource_name       VARCHAR(100) NOT NULL,
    location            VARCHAR(100),
    status              VARCHAR(30) DEFAULT 'Available' CHECK (status IN ('Available', 'In Use', 'Maintenance')),
    assigned_patient_id INTEGER REFERENCES patients(id) ON DELETE SET NULL,
    assigned_at         TIMESTAMP,
    notes               TEXT,
    created_at          TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
```

---

## 6. Frontend Changes

### 6.1 Branding Update
- All instances of "Mediflow" → **"EmergiCare"**
- Subtitle: "Emergency Care Coordination System"
- `index.html` title tag updated

### 6.2 New Navigation Items

| Nav Item        | Roles Allowed       | View Component        |
|-----------------|---------------------|-----------------------|
| Triage          | Admin, Doctor       | `TriageView`          |
| Emergency Queue | Admin, Doctor, Receptionist | `EmergencyQueueView` |
| Resources       | Admin, Doctor       | `ResourceView`        |

### 6.3 Updated Dashboard Stats
Current: Total Patients, Appointments Today, Total Staff
New additions:
- **Active Emergencies** (queue_status = 'Waiting' or 'In Treatment')
- **Critical Patients** (severity_level = 'P1')
- **Available Beds** (resource_type = 'Bed' AND status = 'Available')

### 6.4 New View Components

**TriageView:**
- Dropdown to select patient
- Input fields: Chief Complaint, Heart Rate, Systolic BP, Diastolic BP, Temperature, O2 Saturation
- "Assess" button → POST to `/api/triage`
- Results table with color-coded severity badges

**EmergencyQueueView:**
- Ordered table (P1 rows highlighted red, P2 orange, etc.)
- Columns: Rank, Patient, Severity, Complaint, Wait Time, Status, Doctor
- Actions: "Start Treatment", "Discharge"

**ResourceView:**
- Card grid for each resource type
- Available count vs total count per type
- Allocate/Release action per resource

---

## 7. C++ Project Structure

```
backend/
├── CMakeLists.txt
├── Dockerfile
├── config.json
├── main.cpp
└── src/
    ├── controllers/
    │   ├── AuthController.h
    │   ├── AuthController.cc
    │   ├── UserController.h
    │   ├── UserController.cc
    │   ├── PatientController.h
    │   ├── PatientController.cc
    │   ├── AppointmentController.h
    │   ├── AppointmentController.cc
    │   ├── MedicalRecordController.h
    │   ├── MedicalRecordController.cc
    │   ├── TriageController.h
    │   ├── TriageController.cc
    │   ├── EmergencyQueueController.h
    │   ├── EmergencyQueueController.cc
    │   ├── ResourceController.h
    │   └── ResourceController.cc
    ├── middleware/
    │   ├── AuthFilter.h
    │   └── AuthFilter.cc
    ├── models/
    │   ├── User.h
    │   ├── Patient.h
    │   ├── Appointment.h
    │   ├── MedicalRecord.h
    │   ├── TriageEntry.h
    │   ├── EmergencyQueueEntry.h
    │   └── Resource.h
    ├── services/
    │   ├── TriageService.h
    │   └── TriageService.cc
    └── utils/
        ├── JwtHelper.h
        ├── JwtHelper.cc
        ├── PasswordHelper.h
        └── PasswordHelper.cc
```

---

## 8. Docker Architecture

```
docker-compose.yml
│
├── postgres (postgres:16-alpine)
│   └── Volume: postgres_data
│   └── Init: init-db.sql
│
├── backend (C++ Drogon)
│   ├── Build: Ubuntu 22.04 + CMake
│   ├── Port: 3001:3001
│   └── Depends: postgres
│
└── frontend (React → Nginx)
    ├── Build: Node 20 build → nginx:1.25-alpine serve
    ├── Port: 8080:80
    └── Depends: backend
```

**Backend Dockerfile Strategy (Multi-Stage):**
```dockerfile
# Stage 1: Builder
FROM ubuntu:22.04 AS builder
RUN apt-get install -y cmake g++ libpq-dev ... drogon deps
COPY . /app
RUN cmake -B build && cmake --build build

# Stage 2: Runtime
FROM ubuntu:22.04
COPY --from=builder /app/build/emergicare_server /usr/local/bin/
CMD ["emergicare_server"]
```

---

## 9. API Contract (Complete Reference)

All endpoints return JSON. All protected routes require `Authorization: Bearer <token>` header.

### Auth
| Method | Route       | Auth | Body                              | Response |
|--------|-------------|------|-----------------------------------|----------|
| POST   | /api/login  | No   | `{username, password}`            | `{success, token, user}` |

### Users
| Method | Route       | Auth | Roles | Body |
|--------|-------------|------|-------|------|
| GET    | /api/users  | Yes  | All   | —    |
| POST   | /api/users  | No   | —     | `{username, password, role, name}` |

### Patients
| Method | Route             | Auth | Roles          | Body |
|--------|-------------------|------|----------------|------|
| GET    | /api/patients     | Yes  | All            | —    |
| POST   | /api/patients     | Yes  | Admin/Doctor/Receptionist | patient fields |
| PUT    | /api/patients/:id | Yes  | Admin/Doctor   | patient fields |
| DELETE | /api/patients/:id | Yes  | Admin          | —    |

### Appointments
| Method | Route              | Auth | Body |
|--------|--------------------|------|------|
| GET    | /api/appointments  | Yes  | —    |
| POST   | /api/appointments  | Yes  | `{patientId, patientName, doctorName, date, reason}` |

### Medical Records
| Method | Route                | Auth | Body |
|--------|----------------------|------|------|
| GET    | /api/medical-records | Yes  | —    |
| POST   | /api/medical-records | Yes  | `{patientId, doctorName, date, diagnosis, prescription, notes}` |

### Triage (NEW)
| Method | Route           | Auth | Roles         | Body |
|--------|-----------------|------|---------------|------|
| GET    | /api/triage     | Yes  | All           | —    |
| POST   | /api/triage     | Yes  | Admin/Doctor  | `{patientId, chiefComplaint, heartRate?, systolicBp?, diastolicBp?, temperature?, oxygenSat?, severityOverride?}` |
| GET    | /api/triage/:id | Yes  | All           | —    |
| PUT    | /api/triage/:id | Yes  | Admin/Doctor  | `{status}` |

### Emergency Queue (NEW)
| Method | Route                    | Auth | Body |
|--------|--------------------------|------|------|
| GET    | /api/emergency-queue     | Yes  | —    |
| POST   | /api/emergency-queue     | Yes  | `{triageId, patientId}` |
| PUT    | /api/emergency-queue/:id | Yes  | `{queueStatus, assignedDoctor?, assignedBed?}` |
| DELETE | /api/emergency-queue/:id | Yes  | —    |

### Resources (NEW)
| Method | Route                  | Auth | Body |
|--------|------------------------|------|------|
| GET    | /api/resources         | Yes  | —    |
| GET    | /api/resources/available | Yes | —   |
| POST   | /api/resources         | Yes  | `{resourceType, resourceName, location?}` |
| PUT    | /api/resources/:id     | Yes  | `{status, assignedPatientId?}` |

---

## 10. Implementation Order

1. **Phase 1:** Delete old backend, clean up root-level files
2. **Phase 2:** Set up C++ project skeleton (CMakeLists.txt, Dockerfile, main.cpp)
3. **Phase 3:** Implement auth & RBAC (AuthController, AuthFilter, JwtHelper, PasswordHelper)
4. **Phase 4:** Port existing endpoints (Patient, Appointment, MedicalRecord, User controllers)
5. **Phase 5:** Implement TriageService with function overloads
6. **Phase 6:** Implement new controllers (Triage, EmergencyQueue, Resource)
7. **Phase 7:** Update init-db.sql with new tables + seed data
8. **Phase 8:** Update docker-compose.yml with postgres service
9. **Phase 9:** Update frontend (branding + 3 new views + updated dashboard)
10. **Phase 10:** Build and test end-to-end

---

## 11. Default Credentials (Seeded)

| Role         | Username       | Password           |
|--------------|----------------|--------------------|
| Admin        | admin          | Admin@123          |
| Doctor       | doctor1        | Doctor@123         |
| Receptionist | receptionist1  | Receptionist@123   |

---

## 12. Non-Functional Requirements

- Backend binary must start within 5 seconds inside Docker
- API response time < 200ms for all endpoints under normal load
- JWT tokens expire after 7 days
- All passwords stored as bcrypt hashes (cost factor 10)
- CORS enabled for `http://localhost:8080` (frontend origin)
- PostgreSQL connection pooling via Drogon's built-in `DbClient`

---

## 13. Academic Requirements Checklist

- [x] **C++ as primary language** — entire backend in C++17
- [x] **Function Overloading** — `TriageService::assess()` with 4 signatures
- [x] **OOP** — controllers and models as classes with encapsulation
- [x] **STL Usage** — `std::string`, `std::vector`, `std::map` throughout
- [x] **Memory management** — Drogon uses smart pointers (`std::shared_ptr`)
- [x] **Docker** — isolated containerized deployment
- [x] **Database** — PostgreSQL with real relational schema
- [x] **REST API** — full CRUD for all resources
