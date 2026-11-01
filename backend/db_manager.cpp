#include "db_manager.hpp"
#include <iostream>
#include <sstream>
#include <algorithm>

namespace lifelink {

DatabaseManager::DatabaseManager() : db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

void DatabaseManager::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::executeScript(const std::string& sql) {
    if (!db_) return false;
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Exec failed: " << (errMsg ? errMsg : "unknown") << std::endl;
        if (errMsg) sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool DatabaseManager::init(const std::string& dbPath) {
    db_path_ = dbPath;
    int rc = sqlite3_open(db_path_.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "[DB ERROR] Cannot open database: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    // Enable foreign keys
    executeScript("PRAGMA foreign_keys = ON;");

    // Create tables
    std::string schemaSql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT NOT NULL,
            email TEXT UNIQUE NOT NULL,
            password TEXT NOT NULL,
            phone TEXT NOT NULL,
            role TEXT DEFAULT 'donor',
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );

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

        CREATE TABLE IF NOT EXISTS blood_requests (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            patient_name TEXT NOT NULL,
            blood_group TEXT NOT NULL,
            units_needed INTEGER NOT NULL,
            hospital_name TEXT NOT NULL,
            city TEXT NOT NULL,
            contact_person TEXT NOT NULL,
            contact_phone TEXT NOT NULL,
            urgency TEXT NOT NULL DEFAULT 'Urgent',
            status TEXT DEFAULT 'Pending',
            notes TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";

    if (!executeScript(schemaSql)) {
        std::cerr << "[DB ERROR] Failed to create tables!" << std::endl;
        return false;
    }

    // Seed data if empty
    seedInitialDataIfEmpty();

    return true;
}

void DatabaseManager::seedInitialDataIfEmpty() {
    sqlite3_stmt* stmt = nullptr;
    std::string query = "SELECT COUNT(*) FROM donors;";
    int count = 0;
    if (sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (count == 0) {
        std::cout << "[DB INFO] Seeding initial mock data into database..." << std::endl;
        std::string seedSql = R"(
            INSERT INTO users (id, name, email, password, phone, role) VALUES 
            (1, 'Admin LifeLink', 'admin@lifelink.org', 'admin123', '+91 98765 43210', 'admin'),
            (2, 'Rahul Sharma', 'rahul.sharma@example.com', 'rahul123', '+91 98111 22334', 'donor'),
            (3, 'Priya Patel', 'priya.patel@example.com', 'priya123', '+91 98222 33445', 'donor'),
            (4, 'Ananya Sen', 'ananya.sen@example.com', 'ananya123', '+91 98333 44556', 'donor');

            INSERT INTO donors (id, user_id, name, age, gender, blood_group, phone, email, city, state, is_available, last_donation_date) VALUES
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

            INSERT INTO blood_requests (id, patient_name, blood_group, units_needed, hospital_name, city, contact_person, contact_phone, urgency, status, notes) VALUES
            (1, 'Aarav Mehta', 'O-', 2, 'Apollo Hospital', 'Delhi', 'Suresh Mehta', '+91 91234 56780', 'Critical', 'Pending', 'Emergency accident surgery scheduled today.'),
            (2, 'Sunita Gupta', 'B+', 3, 'Lilavati Hospital', 'Mumbai', 'Rajesh Gupta', '+91 92345 67891', 'Urgent', 'Pending', 'Patient undergoing chemotherapy, urgently needs platelet match.'),
            (3, 'Mohammed Farooq', 'AB-', 1, 'Fortis Memorial', 'Bengaluru', 'Fatima Farooq', '+91 93456 78902', 'Critical', 'Pending', 'Rare blood type needed for open heart surgery.'),
            (4, 'Kavita Chawla', 'A+', 2, 'Manipal Hospital', 'Pune', 'Dr. Alok Verma', '+91 94567 89013', 'Standard', 'Fulfilled', 'Units received successfully. Thank you LifeLink donors!');
        )";
        executeScript(seedSql);
    }
}

// --------------------------------------------------------------------------
// Donor CRUD Operations
// --------------------------------------------------------------------------

std::vector<Donor> DatabaseManager::getDonors(const std::string& bloodGroup, const std::string& city, int availableOnly) {
    std::vector<Donor> result;
    if (!db_) return result;

    std::string sql = "SELECT id, user_id, name, age, gender, blood_group, phone, email, city, state, address, is_available, last_donation_date, created_at FROM donors WHERE 1=1";
    
    if (availableOnly > 0) {
        sql += " AND is_available = 1";
    }
    if (!bloodGroup.empty() && bloodGroup != "ALL") {
        sql += " AND blood_group = ?";
    }
    if (!city.empty() && city != "ALL") {
        sql += " AND LOWER(city) LIKE LOWER(?)";
    }
    sql += " ORDER BY id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB ERROR] getDonors prepare failed: " << sqlite3_errmsg(db_) << std::endl;
        return result;
    }

    int bindIdx = 1;
    if (!bloodGroup.empty() && bloodGroup != "ALL") {
        sqlite3_bind_text(stmt, bindIdx++, bloodGroup.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!city.empty() && city != "ALL") {
        std::string cityWildcard = "%" + city + "%";
        sqlite3_bind_text(stmt, bindIdx++, cityWildcard.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Donor d;
        d.id = sqlite3_column_int(stmt, 0);
        d.user_id = sqlite3_column_int(stmt, 1);
        d.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        d.age = sqlite3_column_int(stmt, 3);
        d.gender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        d.blood_group = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        d.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        d.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : (const unsigned char*)"");
        d.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8) ? sqlite3_column_text(stmt, 8) : (const unsigned char*)"");
        d.state = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9) ? sqlite3_column_text(stmt, 9) : (const unsigned char*)"");
        d.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10) ? sqlite3_column_text(stmt, 10) : (const unsigned char*)"");
        d.is_available = sqlite3_column_int(stmt, 11);
        d.last_donation_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12) ? sqlite3_column_text(stmt, 12) : (const unsigned char*)"");
        d.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13) ? sqlite3_column_text(stmt, 13) : (const unsigned char*)"");
        result.push_back(d);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<Donor> DatabaseManager::getDonorById(int id) {
    if (!db_) return std::nullopt;
    std::string sql = "SELECT id, user_id, name, age, gender, blood_group, phone, email, city, state, address, is_available, last_donation_date, created_at FROM donors WHERE id = ?;";
    
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Donor d;
        d.id = sqlite3_column_int(stmt, 0);
        d.user_id = sqlite3_column_int(stmt, 1);
        d.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        d.age = sqlite3_column_int(stmt, 3);
        d.gender = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        d.blood_group = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        d.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        d.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : (const unsigned char*)"");
        d.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8) ? sqlite3_column_text(stmt, 8) : (const unsigned char*)"");
        d.state = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9) ? sqlite3_column_text(stmt, 9) : (const unsigned char*)"");
        d.address = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10) ? sqlite3_column_text(stmt, 10) : (const unsigned char*)"");
        d.is_available = sqlite3_column_int(stmt, 11);
        d.last_donation_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 12) ? sqlite3_column_text(stmt, 12) : (const unsigned char*)"");
        d.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 13) ? sqlite3_column_text(stmt, 13) : (const unsigned char*)"");
        sqlite3_finalize(stmt);
        return d;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool DatabaseManager::addDonor(const Donor& d, int* outId) {
    if (!db_) return false;
    std::string sql = "INSERT INTO donors (user_id, name, age, gender, blood_group, phone, email, city, state, address, is_available, last_donation_date) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB ERROR] addDonor prepare failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    if (d.user_id > 0) {
        sqlite3_bind_int(stmt, 1, d.user_id);
    } else {
        sqlite3_bind_null(stmt, 1);
    }
    sqlite3_bind_text(stmt, 2, d.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, d.age);
    sqlite3_bind_text(stmt, 4, d.gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, d.blood_group.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, d.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, d.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, d.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, d.state.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, d.address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 11, d.is_available);
    sqlite3_bind_text(stmt, 12, d.last_donation_date.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        if (outId) {
            *outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        }
        return true;
    }
    return false;
}

bool DatabaseManager::updateDonor(const Donor& d) {
    if (!db_ || d.id <= 0) return false;
    std::string sql = "UPDATE donors SET name=?, age=?, gender=?, blood_group=?, phone=?, email=?, city=?, state=?, address=?, is_available=?, last_donation_date=? WHERE id=?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, d.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, d.age);
    sqlite3_bind_text(stmt, 3, d.gender.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, d.blood_group.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, d.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, d.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, d.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, d.state.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, d.address.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 10, d.is_available);
    sqlite3_bind_text(stmt, 11, d.last_donation_date.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 12, d.id);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::updateDonorAvailability(int donorId, int isAvailable) {
    if (!db_) return false;
    std::string sql = "UPDATE donors SET is_available = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, isAvailable);
    sqlite3_bind_int(stmt, 2, donorId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::deleteDonor(int donorId) {
    if (!db_) return false;
    std::string sql = "DELETE FROM donors WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, donorId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// --------------------------------------------------------------------------
// Blood Request CRUD Operations
// --------------------------------------------------------------------------

std::vector<BloodRequest> DatabaseManager::getBloodRequests(const std::string& status, const std::string& bloodGroup, const std::string& city) {
    std::vector<BloodRequest> result;
    if (!db_) return result;

    std::string sql = "SELECT id, patient_name, blood_group, units_needed, hospital_name, city, contact_person, contact_phone, urgency, status, notes, created_at FROM blood_requests WHERE 1=1";

    if (!status.empty() && status != "ALL") {
        sql += " AND status = ?";
    }
    if (!bloodGroup.empty() && bloodGroup != "ALL") {
        sql += " AND blood_group = ?";
    }
    if (!city.empty() && city != "ALL") {
        sql += " AND LOWER(city) LIKE LOWER(?)";
    }
    sql += " ORDER BY CASE urgency WHEN 'Critical' THEN 1 WHEN 'Urgent' THEN 2 ELSE 3 END, id DESC;";

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB ERROR] getBloodRequests prepare failed: " << sqlite3_errmsg(db_) << std::endl;
        return result;
    }

    int bindIdx = 1;
    if (!status.empty() && status != "ALL") {
        sqlite3_bind_text(stmt, bindIdx++, status.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!bloodGroup.empty() && bloodGroup != "ALL") {
        sqlite3_bind_text(stmt, bindIdx++, bloodGroup.c_str(), -1, SQLITE_TRANSIENT);
    }
    if (!city.empty() && city != "ALL") {
        std::string cityWildcard = "%" + city + "%";
        sqlite3_bind_text(stmt, bindIdx++, cityWildcard.c_str(), -1, SQLITE_TRANSIENT);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        BloodRequest r;
        r.id = sqlite3_column_int(stmt, 0);
        r.patient_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : (const unsigned char*)"");
        r.blood_group = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        r.units_needed = sqlite3_column_int(stmt, 3);
        r.hospital_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        r.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        r.contact_person = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        r.contact_phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : (const unsigned char*)"");
        r.urgency = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8) ? sqlite3_column_text(stmt, 8) : (const unsigned char*)"");
        r.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9) ? sqlite3_column_text(stmt, 9) : (const unsigned char*)"");
        r.notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10) ? sqlite3_column_text(stmt, 10) : (const unsigned char*)"");
        r.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11) ? sqlite3_column_text(stmt, 11) : (const unsigned char*)"");
        result.push_back(r);
    }

    sqlite3_finalize(stmt);
    return result;
}

std::optional<BloodRequest> DatabaseManager::getBloodRequestById(int id) {
    if (!db_) return std::nullopt;
    std::string sql = "SELECT id, patient_name, blood_group, units_needed, hospital_name, city, contact_person, contact_phone, urgency, status, notes, created_at FROM blood_requests WHERE id = ?;";
    
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        BloodRequest r;
        r.id = sqlite3_column_int(stmt, 0);
        r.patient_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : (const unsigned char*)"");
        r.blood_group = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        r.units_needed = sqlite3_column_int(stmt, 3);
        r.hospital_name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        r.city = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        r.contact_person = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        r.contact_phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 7) ? sqlite3_column_text(stmt, 7) : (const unsigned char*)"");
        r.urgency = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 8) ? sqlite3_column_text(stmt, 8) : (const unsigned char*)"");
        r.status = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 9) ? sqlite3_column_text(stmt, 9) : (const unsigned char*)"");
        r.notes = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 10) ? sqlite3_column_text(stmt, 10) : (const unsigned char*)"");
        r.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 11) ? sqlite3_column_text(stmt, 11) : (const unsigned char*)"");
        sqlite3_finalize(stmt);
        return r;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool DatabaseManager::addBloodRequest(const BloodRequest& req, int* outId) {
    if (!db_) return false;
    std::string sql = "INSERT INTO blood_requests (patient_name, blood_group, units_needed, hospital_name, city, contact_person, contact_phone, urgency, status, notes) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "[DB ERROR] addBloodRequest prepare failed: " << sqlite3_errmsg(db_) << std::endl;
        return false;
    }

    sqlite3_bind_text(stmt, 1, req.patient_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, req.blood_group.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 3, req.units_needed);
    sqlite3_bind_text(stmt, 4, req.hospital_name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, req.city.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 6, req.contact_person.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 7, req.contact_phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 8, req.urgency.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 9, req.status.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 10, req.notes.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        if (outId) {
            *outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        }
        return true;
    }
    return false;
}

bool DatabaseManager::updateBloodRequestStatus(int requestId, const std::string& newStatus) {
    if (!db_) return false;
    std::string sql = "UPDATE blood_requests SET status = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_text(stmt, 1, newStatus.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, requestId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

bool DatabaseManager::deleteBloodRequest(int requestId) {
    if (!db_) return false;
    std::string sql = "DELETE FROM blood_requests WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, requestId);
    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return (rc == SQLITE_DONE);
}

// --------------------------------------------------------------------------
// Statistics
// --------------------------------------------------------------------------

Statistics DatabaseManager::getStatistics() {
    Statistics stats;
    if (!db_) return stats;

    sqlite3_stmt* stmt = nullptr;

    // Total donors & available donors
    std::string qDonors = "SELECT COUNT(*), SUM(CASE WHEN is_available = 1 THEN 1 ELSE 0 END) FROM donors;";
    if (sqlite3_prepare_v2(db_, qDonors.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.total_donors = sqlite3_column_int(stmt, 0);
            stats.available_donors = sqlite3_column_int(stmt, 1);
        }
        sqlite3_finalize(stmt);
    }

    // Requests counts
    std::string qReqs = "SELECT COUNT(*), "
                        "SUM(CASE WHEN status = 'Pending' THEN 1 ELSE 0 END), "
                        "SUM(CASE WHEN status = 'Fulfilled' THEN 1 ELSE 0 END), "
                        "SUM(CASE WHEN status = 'Pending' THEN units_needed ELSE 0 END) "
                        "FROM blood_requests;";
    if (sqlite3_prepare_v2(db_, qReqs.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats.total_requests = sqlite3_column_int(stmt, 0);
            stats.pending_requests = sqlite3_column_int(stmt, 1);
            stats.fulfilled_requests = sqlite3_column_int(stmt, 2);
            stats.total_units_needed = sqlite3_column_int(stmt, 3);
        }
        sqlite3_finalize(stmt);
    }

    // Standard blood groups breakdown
    const std::vector<std::string> groups = {"A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-"};
    for (const auto& bg : groups) {
        BloodGroupStat bStat;
        bStat.blood_group = bg;
        bStat.count = 0;

        std::string qBg = "SELECT COUNT(*) FROM donors WHERE blood_group = ? AND is_available = 1;";
        if (sqlite3_prepare_v2(db_, qBg.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
            sqlite3_bind_text(stmt, 1, bg.c_str(), -1, SQLITE_TRANSIENT);
            if (sqlite3_step(stmt) == SQLITE_ROW) {
                bStat.count = sqlite3_column_int(stmt, 0);
            }
            sqlite3_finalize(stmt);
        }
        stats.blood_groups.push_back(bStat);
    }

    return stats;
}

// --------------------------------------------------------------------------
// User Authentication Operations
// --------------------------------------------------------------------------

bool DatabaseManager::registerUser(const User& user, int* outId) {
    if (!db_) return false;
    std::string sql = "INSERT INTO users (name, email, password, phone, role) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_text(stmt, 1, user.name.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, user.password.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, user.phone.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 5, user.role.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    if (rc == SQLITE_DONE) {
        if (outId) {
            *outId = static_cast<int>(sqlite3_last_insert_rowid(db_));
        }
        return true;
    }
    return false;
}

std::optional<User> DatabaseManager::authenticateUser(const std::string& email, const std::string& password) {
    if (!db_) return std::nullopt;
    std::string sql = "SELECT id, name, email, password, phone, role, created_at FROM users WHERE LOWER(email) = LOWER(?) AND password = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, password.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : (const unsigned char*)"");
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : (const unsigned char*)"");
        u.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        sqlite3_finalize(stmt);
        return u;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> DatabaseManager::getUserById(int id) {
    if (!db_) return std::nullopt;
    std::string sql = "SELECT id, name, email, password, phone, role, created_at FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : (const unsigned char*)"");
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : (const unsigned char*)"");
        u.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        sqlite3_finalize(stmt);
        return u;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::optional<User> DatabaseManager::getUserByEmail(const std::string& email) {
    if (!db_) return std::nullopt;
    std::string sql = "SELECT id, name, email, password, phone, role, created_at FROM users WHERE LOWER(email) = LOWER(?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }

    sqlite3_bind_text(stmt, 1, email.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        User u;
        u.id = sqlite3_column_int(stmt, 0);
        u.name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1) ? sqlite3_column_text(stmt, 1) : (const unsigned char*)"");
        u.email = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2) ? sqlite3_column_text(stmt, 2) : (const unsigned char*)"");
        u.password = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3) ? sqlite3_column_text(stmt, 3) : (const unsigned char*)"");
        u.phone = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4) ? sqlite3_column_text(stmt, 4) : (const unsigned char*)"");
        u.role = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5) ? sqlite3_column_text(stmt, 5) : (const unsigned char*)"");
        u.created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6) ? sqlite3_column_text(stmt, 6) : (const unsigned char*)"");
        sqlite3_finalize(stmt);
        return u;
    }

    sqlite3_finalize(stmt);
    return std::nullopt;
}

} // namespace lifelink
