-- PostgreSQL Healthcare Database Schema
-- For local development with PostgreSQL
-- Create users table
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    role VARCHAR(50) NOT NULL CHECK (role IN ('Admin', 'Doctor', 'Receptionist')),
    name VARCHAR(100) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- Create patients table
CREATE TABLE IF NOT EXISTS patients (
    id SERIAL PRIMARY KEY,
    name VARCHAR(100) NOT NULL,
    dob DATE,
    gender VARCHAR(10),
    contact VARCHAR(20),
    email VARCHAR(100),
    address TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- Create appointments table
CREATE TABLE IF NOT EXISTS appointments (
    id SERIAL PRIMARY KEY,
    patient_id INTEGER REFERENCES patients(id) ON DELETE
    SET NULL,
        patient_name VARCHAR(100),
        doctor_name VARCHAR(100),
        date TIMESTAMP,
        reason TEXT,
        status VARCHAR(50) DEFAULT 'Scheduled',
        created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- Create medical_records table
CREATE TABLE IF NOT EXISTS medical_records (
    id SERIAL PRIMARY KEY,
    patient_id INTEGER REFERENCES patients(id) ON DELETE CASCADE,
    doctor_name VARCHAR(100),
    date DATE,
    diagnosis TEXT,
    prescription TEXT,
    notes TEXT,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);
-- Insert sample admin user (password: Admin@123 - hashed with bcrypt)
-- To create users, use the POST /api/users endpoint with your desired password
INSERT INTO users (username, password, role, name)
VALUES (
        'admin',
        '$2a$10$fJINTudFfzFdPBMdupOAUe/7UXqpVTY.bYc2f2ZS.j.9XWhcstNfi',
        'Admin',
        'Administrator'
    ) ON CONFLICT (username) DO NOTHING;
-- Insert sample doctor user (password: Doctor@123)
INSERT INTO users (username, password, role, name)
VALUES (
        'doctor1',
        '$2a$10$95jcByO/UGyWkYXIyvIcW.w/rt34awyegjuVKY8VGJKG3TjJDu8Ry',
        'Doctor',
        'Dr. John Smith'
    ) ON CONFLICT (username) DO NOTHING;
-- Insert sample receptionist user (password: Receptionist@123)
INSERT INTO users (username, password, role, name)
VALUES (
        'receptionist1',
        '$2a$10$LCglfJHv2jpk6c2qnWM2sOggrPUYl7sSU.Q77YLhweN.uOWeG73se',
        'Receptionist',
        'Jane Doe'
    ) ON CONFLICT (username) DO NOTHING;
-- Insert sample patient
INSERT INTO patients (name, dob, gender, contact, email, address)
VALUES (
        'Michael Johnson',
        '1990-05-15',
        'Male',
        '+1234567890',
        'michael@email.com',
        '123 Main St, City'
    ) ON CONFLICT DO NOTHING;
COMMIT;