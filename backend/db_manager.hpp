#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <mutex>
#include "sqlite3.h"

namespace lifelink {

// User model for authentication
struct User {
    int id = 0;
    std::string name;
    std::string email;
    std::string password;
    std::string phone;
    std::string role = "donor";
    std::string created_at;
};

// Donor model
struct Donor {
    int id = 0;
    int user_id = 0;
    std::string name;
    int age = 0;
    std::string gender = "Unspecified";
    std::string blood_group;
    std::string phone;
    std::string email;
    std::string city;
    std::string state;
    std::string address;
    int is_available = 1;
    std::string last_donation_date;
    std::string created_at;
};

// Blood request model
struct BloodRequest {
    int id = 0;
    std::string patient_name;
    std::string blood_group;
    int units_needed = 1;
    std::string hospital_name;
    std::string city;
    std::string contact_person;
    std::string contact_phone;
    std::string urgency = "Urgent"; // "Critical", "Urgent", "Standard"
    std::string status = "Pending";  // "Pending", "Fulfilled", "Cancelled"
    std::string notes;
    std::string created_at;
};

// Blood group count item
struct BloodGroupStat {
    std::string blood_group;
    int count = 0;
};

// Overall system statistics
struct Statistics {
    int total_donors = 0;
    int available_donors = 0;
    int total_requests = 0;
    int pending_requests = 0;
    int fulfilled_requests = 0;
    int total_units_needed = 0;
    std::vector<BloodGroupStat> blood_groups;
};

// Database Manager class wrapping SQLite operations
class DatabaseManager {
public:
    DatabaseManager();
    ~DatabaseManager();

    // Initialize database connection and schemas
    bool init(const std::string& dbPath = "database/lifelink.db");
    void close();

    // Donor CRUD operations
    std::vector<Donor> getDonors(const std::string& bloodGroup = "", const std::string& city = "", int availableOnly = 0);
    std::optional<Donor> getDonorById(int id);
    bool addDonor(const Donor& donor, int* outId = nullptr);
    bool updateDonor(const Donor& donor);
    bool updateDonorAvailability(int donorId, int isAvailable);
    bool deleteDonor(int donorId);

    // Blood Request CRUD operations
    std::vector<BloodRequest> getBloodRequests(const std::string& status = "", const std::string& bloodGroup = "", const std::string& city = "");
    std::optional<BloodRequest> getBloodRequestById(int id);
    bool addBloodRequest(const BloodRequest& req, int* outId = nullptr);
    bool updateBloodRequestStatus(int requestId, const std::string& newStatus);
    bool deleteBloodRequest(int requestId);

    // Dashboard Statistics
    Statistics getStatistics();

    // Authentication & Users
    bool registerUser(const User& user, int* outId = nullptr);
    std::optional<User> authenticateUser(const std::string& email, const std::string& password);
    std::optional<User> getUserById(int id);
    std::optional<User> getUserByEmail(const std::string& email);

private:
    sqlite3* db_ = nullptr;
    std::string db_path_;
    mutable std::recursive_mutex db_mutex_;

    bool executeScript(const std::string& sql);
    void seedInitialDataIfEmpty();
};

} // namespace lifelink
