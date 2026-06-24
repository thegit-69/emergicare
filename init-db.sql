-- ================================================================
-- EmergiCare – PostgreSQL Database Schema
-- Version: 2.0 (Emergency Care Coordination System)
-- ================================================================
-- Safe to re-run: all tables use IF NOT EXISTS, inserts use ON CONFLICT DO NOTHING

-- ================================================================
-- SECTION 1: EXISTING TABLES (Preserved from v1.0)
-- ================================================================

-- Staff accounts with role-based access control
CREATE TABLE IF NOT EXISTS users (
    id         SERIAL PRIMARY KEY,
    username   VARCHAR(50)  UNIQUE NOT NULL,
    password   VARCHAR(255) NOT NULL,
    role       VARCHAR(50)  NOT NULL CHECK (role IN ('Admin', 'Doctor', 'Nurse', 'Receptionist')),
    name       VARCHAR(100) NOT NULL,
    created_at TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Patient demographic records
CREATE TABLE IF NOT EXISTS patients (
    id         SERIAL PRIMARY KEY,
    name       VARCHAR(100) NOT NULL,
    dob        DATE,
    gender     VARCHAR(10),
    contact    VARCHAR(20),
    email      VARCHAR(100),
    address    TEXT,
    created_at TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Scheduled appointments
CREATE TABLE IF NOT EXISTS appointments (
    id           SERIAL PRIMARY KEY,
    patient_id   INTEGER REFERENCES patients(id) ON DELETE SET NULL,
    patient_name VARCHAR(100),
    doctor_name  VARCHAR(100),
    date         TIMESTAMP,
    reason       TEXT,
    status       VARCHAR(50) NOT NULL DEFAULT 'Scheduled'
                 CHECK (status IN ('Scheduled', 'Completed', 'Cancelled')),
    created_at   TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Clinical diagnosis and prescription records
CREATE TABLE IF NOT EXISTS medical_records (
    id           SERIAL PRIMARY KEY,
    patient_id   INTEGER REFERENCES patients(id) ON DELETE CASCADE,
    doctor_name  VARCHAR(100),
    date         DATE,
    diagnosis    TEXT,
    prescription TEXT,
    notes        TEXT,
    created_at   TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ================================================================
-- SECTION 2: NEW EMERGENCY TABLES (v2.0)
-- ================================================================

-- Triage assessment records
-- Linked to the C++ TriageService::assess() function overloads
CREATE TABLE IF NOT EXISTS triage_entries (
    id              SERIAL PRIMARY KEY,
    patient_id      INTEGER     NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    severity_level  VARCHAR(5)  NOT NULL CHECK (severity_level IN ('P1', 'P2', 'P3', 'P4')),
    severity_label  VARCHAR(20) NOT NULL,  -- "Immediate" | "Urgent" | "Less Urgent" | "Non-Urgent"
    chief_complaint TEXT,                  -- Used by assess(std::string) overload
    heart_rate      INTEGER,               -- Used by assess(int,int,int) overload
    systolic_bp     INTEGER,
    diastolic_bp    INTEGER,
    temperature     DECIMAL(4,1),
    oxygen_sat      INTEGER,               -- O2 saturation %
    assessed_by     VARCHAR(100),          -- Doctor name from JWT claims
    notes           TEXT,
    status          VARCHAR(30) NOT NULL DEFAULT 'Waiting'
                    CHECK (status IN ('Waiting', 'In Treatment', 'Discharged')),
    assessed_at     TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- Live emergency priority queue
-- Ordered by priority_score ASC (1=P1 critical) then enqueued_at ASC (FIFO)
CREATE TABLE IF NOT EXISTS emergency_queue (
    id                   SERIAL PRIMARY KEY,
    triage_id            INTEGER     NOT NULL REFERENCES triage_entries(id) ON DELETE CASCADE,
    patient_id           INTEGER     NOT NULL REFERENCES patients(id) ON DELETE CASCADE,
    priority_score       INTEGER     NOT NULL CHECK (priority_score BETWEEN 1 AND 4),
    queue_status         VARCHAR(30) NOT NULL DEFAULT 'Waiting'
                         CHECK (queue_status IN ('Waiting', 'In Treatment', 'Discharged')),
    assigned_doctor      VARCHAR(100),
    assigned_bed         VARCHAR(20),
    enqueued_at          TIMESTAMP   NOT NULL DEFAULT CURRENT_TIMESTAMP,
    treatment_started_at TIMESTAMP,        -- Set when status → 'In Treatment'
    discharged_at        TIMESTAMP         -- Set when status → 'Discharged'
);

-- Hospital resource inventory and availability tracking
CREATE TABLE IF NOT EXISTS resources (
    id                  SERIAL PRIMARY KEY,
    resource_type       VARCHAR(50)  NOT NULL
                        CHECK (resource_type IN ('Bed', 'Ventilator', 'OperatingRoom', 'Staff')),
    resource_name       VARCHAR(100) NOT NULL,
    location            VARCHAR(100),
    status              VARCHAR(30)  NOT NULL DEFAULT 'Available'
                        CHECK (status IN ('Available', 'In Use', 'Maintenance')),
    assigned_patient_id INTEGER REFERENCES patients(id) ON DELETE SET NULL,
    assigned_at         TIMESTAMP,
    notes               TEXT,
    created_at          TIMESTAMP    NOT NULL DEFAULT CURRENT_TIMESTAMP
);

-- ================================================================
-- SECTION 3: INDEXES (for query performance)
-- ================================================================

CREATE INDEX IF NOT EXISTS idx_triage_severity  ON triage_entries(severity_level);
CREATE INDEX IF NOT EXISTS idx_triage_status    ON triage_entries(status);
CREATE INDEX IF NOT EXISTS idx_triage_patient   ON triage_entries(patient_id);
CREATE INDEX IF NOT EXISTS idx_queue_score      ON emergency_queue(priority_score, enqueued_at);
CREATE INDEX IF NOT EXISTS idx_queue_status     ON emergency_queue(queue_status);
CREATE INDEX IF NOT EXISTS idx_resources_type   ON resources(resource_type);
CREATE INDEX IF NOT EXISTS idx_resources_status ON resources(status);

-- ================================================================
-- SECTION 4: SEED DATA
-- ================================================================

-- -----------------------------------------------
-- Staff Users
-- Passwords hashed with bcrypt cost factor 10.
-- Compatible with both Node.js bcryptjs and C++ libxcrypt.
-- -----------------------------------------------

-- admin / Admin@123
INSERT INTO users (username, password, role, name)
VALUES ('admin', '$2a$10$fJINTudFfzFdPBMdupOAUe/7UXqpVTY.bYc2f2ZS.j.9XWhcstNfi', 'Admin', 'Administrator')
ON CONFLICT (username) DO NOTHING;

-- doctor1 / Doctor@123
INSERT INTO users (username, password, role, name)
VALUES ('doctor1', '$2a$10$95jcByO/UGyWkYXIyvIcW.w/rt34awyegjuVKY8VGJKG3TjJDu8Ry', 'Doctor', 'Dr. John Smith')
ON CONFLICT (username) DO NOTHING;

-- receptionist1 / Receptionist@123
INSERT INTO users (username, password, role, name)
VALUES ('receptionist1', '$2a$10$LCglfJHv2jpk6c2qnWM2sOggrPUYl7sSU.Q77YLhweN.uOWeG73se', 'Receptionist', 'Jane Doe')
ON CONFLICT (username) DO NOTHING;

-- -----------------------------------------------
-- Sample Patients
-- -----------------------------------------------

INSERT INTO patients (name, dob, gender, contact, email, address)
VALUES
    ('Michael Johnson', '1990-05-15', 'Male',   '+1234567890', 'michael@email.com', '123 Main St, City'),
    ('Sarah Williams',  '1985-08-22', 'Female', '+0987654321', 'sarah@email.com',   '456 Oak Ave, Town'),
    ('Robert Chen',     '1972-03-10', 'Male',   '+1122334455', 'robert@email.com',  '789 Pine Rd, Village')
ON CONFLICT DO NOTHING;

-- -----------------------------------------------
-- Sample Appointments
-- -----------------------------------------------

INSERT INTO appointments (patient_id, patient_name, doctor_name, date, reason, status)
SELECT
    p.id, p.name, 'Dr. John Smith',
    NOW() + INTERVAL '1 day', 'Annual checkup', 'Scheduled'
FROM patients p WHERE p.name = 'Michael Johnson'
ON CONFLICT DO NOTHING;

-- -----------------------------------------------
-- Hospital Resources (Emergency Department)
-- -----------------------------------------------

INSERT INTO resources (resource_type, resource_name, location, status, notes) VALUES
    -- Emergency Beds
    ('Bed', 'ER-Bed-01', 'Emergency Room Bay 1', 'Available', 'Standard emergency bed'),
    ('Bed', 'ER-Bed-02', 'Emergency Room Bay 2', 'Available', 'Standard emergency bed'),
    ('Bed', 'ER-Bed-03', 'Emergency Room Bay 3', 'Available', 'Trauma bed'),
    ('Bed', 'ER-Bed-04', 'Emergency Room Bay 4', 'Available', 'Standard emergency bed'),
    ('Bed', 'ER-Bed-05', 'Emergency Room Bay 5', 'Available', 'Isolation bed'),
    ('Bed', 'ICU-Bed-01', 'ICU Unit A',           'Available', 'Intensive care bed'),
    ('Bed', 'ICU-Bed-02', 'ICU Unit A',           'Available', 'Intensive care bed'),

    -- Ventilators
    ('Ventilator', 'Vent-01', 'Emergency Room', 'Available', 'Hamilton C6 mechanical ventilator'),
    ('Ventilator', 'Vent-02', 'Emergency Room', 'Available', 'Hamilton C6 mechanical ventilator'),
    ('Ventilator', 'Vent-03', 'ICU Unit A',     'Available', 'Transport ventilator'),

    -- Operating Rooms
    ('OperatingRoom', 'OR-1', 'Surgical Wing Floor 2', 'Available', 'General surgery suite'),
    ('OperatingRoom', 'OR-2', 'Surgical Wing Floor 2', 'Available', 'Trauma surgery suite'),

    -- On-call Staff
    ('Staff', 'On-Call Cardiologist',  'Emergency Room', 'Available', 'Dr. Emily Rodriguez'),
    ('Staff', 'On-Call Neurologist',   'Emergency Room', 'Available', 'Dr. David Park'),
    ('Staff', 'On-Call Surgeon',       'Surgical Wing',  'Available', 'Dr. Maria Santos'),
    ('Staff', 'Emergency Nurse RN-01', 'Emergency Room', 'Available', 'Alice Thompson'),
    ('Staff', 'Emergency Nurse RN-02', 'Emergency Room', 'Available', 'Bob Martinez')
ON CONFLICT DO NOTHING;

-- ================================================================
-- END OF SCHEMA
-- ================================================================