-- =======================================================
-- LifeLink - Blood Donor Finder Database Schema (SQLite)
-- B.Tech Internship Project
-- =======================================================

PRAGMA foreign_keys = ON;

-- Users Table (for Authentication)
CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    name TEXT NOT NULL,
    email TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL,
    phone TEXT NOT NULL,
    role TEXT DEFAULT 'donor',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Donors Table
CREATE TABLE IF NOT EXISTS donors (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id INTEGER NULL,
    name TEXT NOT NULL,
    age INTEGER NOT NULL,
    gender TEXT NOT NULL,
    blood_group TEXT NOT NULL,
    phone TEXT NOT NULL,
    email TEXT,
    city TEXT NOT NULL,
    state TEXT,
    address TEXT,
    is_available INTEGER DEFAULT 1,
    last_donation_date TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY(user_id) REFERENCES users(id) ON DELETE SET NULL
);

-- Blood Requests Table
CREATE TABLE IF NOT EXISTS blood_requests (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_name TEXT NOT NULL,
    blood_group TEXT NOT NULL,
    units_needed INTEGER NOT NULL,
    hospital_name TEXT NOT NULL,
    city TEXT NOT NULL,
    contact_person TEXT NOT NULL,
    contact_phone TEXT NOT NULL,
    urgency TEXT NOT NULL DEFAULT 'Urgent', -- 'Critical', 'Urgent', 'Standard'
    status TEXT DEFAULT 'Pending',          -- 'Pending', 'Fulfilled', 'Cancelled'
    notes TEXT,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

-- Pre-seed some initial data if empty
INSERT OR IGNORE INTO users (id, name, email, password, phone, role) VALUES 
(1, 'Admin LifeLink', 'admin@lifelink.org', 'admin123', '+91 98765 43210', 'admin'),
(2, 'Rahul Sharma', 'rahul.sharma@example.com', 'rahul123', '+91 98111 22334', 'donor'),
(3, 'Priya Patel', 'priya.patel@example.com', 'priya123', '+91 98222 33445', 'donor'),
(4, 'Ananya Sen', 'ananya.sen@example.com', 'ananya123', '+91 98333 44556', 'donor');

INSERT OR IGNORE INTO donors (id, user_id, name, age, gender, blood_group, phone, email, city, state, is_available, last_donation_date) VALUES
(1, 2, 'Rahul Sharma', 26, 'Male', 'O+', '+91 98111 22334', 'rahul.sharma@example.com', 'Delhi', 'Delhi', 1, '2026-06-15'),
(2, 3, 'Priya Patel', 24, 'Female', 'A+', '+91 98222 33445', 'priya.patel@example.com', 'Mumbai', 'Maharashtra', 1, '2026-08-10'),
(3, 4, 'Ananya Sen', 29, 'Female', 'B+', '+91 98333 44556', 'ananya.sen@example.com', 'Kolkata', 'West Bengal', 1, '2026-07-22'),
(4, NULL, 'Vikram Malhotra', 34, 'Male', 'AB+', '+91 98444 55667', 'vikram.m@example.com', 'Bengaluru', 'Karnataka', 1, '2026-05-18'),
(5, NULL, 'Sneha Reddy', 28, 'Female', 'O-', '+91 98555 66778', 'sneha.r@example.com', 'Hyderabad', 'Telangana', 1, '2026-09-01'),
(6, NULL, 'Amit Verma', 31, 'Male', 'A-', '+91 98666 77889', 'amit.v@example.com', 'Chennai', 'Tamil Nadu', 1, '2026-04-12'),
(7, NULL, 'Rohan Joshi', 23, 'Male', 'B-', '+91 98777 88990', 'rohan.j@example.com', 'Pune', 'Maharashtra', 1, '2026-08-30'),
(8, NULL, 'Deepika Nair', 27, 'Female', 'AB-', '+91 98888 99001', 'deepika.n@example.com', 'Kochi', 'Kerala', 1, '2026-07-14'),
(9, NULL, 'Karan Singhania', 35, 'Male', 'O+', '+91 98999 00112', 'karan.s@example.com', 'Delhi', 'Delhi', 1, '2026-03-25'),
(10, NULL, 'Meera Iyer', 25, 'Female', 'A+', '+91 99000 11223', 'meera.i@example.com', 'Bengaluru', 'Karnataka', 1, '2026-09-15');

INSERT OR IGNORE INTO blood_requests (id, patient_name, blood_group, units_needed, hospital_name, city, contact_person, contact_phone, urgency, status, notes) VALUES
(1, 'Aarav Mehta', 'O-', 2, 'Apollo Hospital', 'Delhi', 'Suresh Mehta', '+91 91234 56780', 'Critical', 'Pending', 'Emergency accident surgery scheduled today.'),
(2, 'Sunita Gupta', 'B+', 3, 'Lilavati Hospital', 'Mumbai', 'Rajesh Gupta', '+91 92345 67891', 'Urgent', 'Pending', 'Patient undergoing chemotherapy, urgently needs platelet match.'),
(3, 'Mohammed Farooq', 'AB-', 1, 'Fortis Memorial', 'Bengaluru', 'Fatima Farooq', '+91 93456 78902', 'Critical', 'Pending', 'Rare blood type needed for open heart surgery.'),
(4, 'Kavita Chawla', 'A+', 2, 'Manipal Hospital', 'Pune', 'Dr. Alok Verma', '+91 94567 89013', 'Standard', 'Fulfilled', 'Units received successfully. Thank you LifeLink donors!');
