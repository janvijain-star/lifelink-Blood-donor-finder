#define CROW_MAIN
#include "crow_all.h"
#include "db_manager.hpp"

#ifdef DELETE
#undef DELETE
#endif

#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;
using namespace lifelink;

// Helper to determine static file root (checks current dir or parent dir)
std::string getFrontendDir() {
    if (fs::exists("frontend/index.html")) {
        return "frontend";
    }
    if (fs::exists("../frontend/index.html")) {
        return "../frontend";
    }
    return "frontend";
}

std::string getDatabasePath() {
    const char* envDb = std::getenv("DATABASE_PATH");
    if (envDb && std::string(envDb).length() > 0) {
        fs::path p(envDb);
        if (p.has_parent_path()) {
            std::error_code ec;
            fs::create_directories(p.parent_path(), ec);
        }
        return envDb;
    }
    if (fs::exists("database")) {
        return "database/lifelink.db";
    }
    if (fs::exists("../database")) {
        return "../database/lifelink.db";
    }
    std::error_code ec;
    fs::create_directories("database", ec);
    return "database/lifelink.db";
}

// Helper to deduce MIME types
std::string getMimeType(const std::string& path) {
    if (path.rfind(".html") != std::string::npos) return "text/html; charset=utf-8";
    if (path.rfind(".css") != std::string::npos) return "text/css; charset=utf-8";
    if (path.rfind(".js") != std::string::npos) return "application/javascript; charset=utf-8";
    if (path.rfind(".svg") != std::string::npos) return "image/svg+xml";
    if (path.rfind(".png") != std::string::npos) return "image/png";
    if (path.rfind(".jpg") != std::string::npos || path.rfind(".jpeg") != std::string::npos) return "image/jpeg";
    if (path.rfind(".ico") != std::string::npos) return "image/x-icon";
    if (path.rfind(".json") != std::string::npos) return "application/json; charset=utf-8";
    return "text/plain; charset=utf-8";
}

// Helper to serve files from disk
crow::response serveFile(const std::string& filePath) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        crow::response res(404, "File Not Found");
        res.add_header("Access-Control-Allow-Origin", "*");
        return res;
    }
    std::ostringstream ss;
    ss << file.rdbuf();
    std::string body = ss.str();

    crow::response res(200, body);
    res.add_header("Content-Type", getMimeType(filePath));
    res.add_header("Access-Control-Allow-Origin", "*");
    return res;
}

// Add CORS headers to JSON responses
crow::response jsonResponse(int code, const crow::json::wvalue& body) {
    crow::response res(code, body.dump());
    res.add_header("Content-Type", "application/json");
    res.add_header("Access-Control-Allow-Origin", "*");
    res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
    return res;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    const char* envPort = std::getenv("PORT");
    if (envPort) {
        port = std::stoi(envPort);
    } else if (argc > 1) {
        port = std::stoi(argv[1]);
    }

    std::cout << "======================================================" << std::endl;
    std::cout << "   LifeLink - Blood Donor Finder Backend Server      " << std::endl;
    std::cout << "   C++ Backend powered by Crow & SQLite3             " << std::endl;
    std::cout << "======================================================" << std::endl;

    std::string dbPath = getDatabasePath();
    std::cout << "[INFO] Initializing SQLite database at: " << dbPath << std::endl;
    DatabaseManager db;
    if (!db.init(dbPath)) {
        std::cerr << "[FATAL] Failed to initialize database! Exiting..." << std::endl;
        return 1;
    }
    std::cout << "[INFO] Database initialized successfully." << std::endl;

    crow::SimpleApp app;
    std::string frontendPath = getFrontendDir();
    std::cout << "[INFO] Serving frontend assets from: " << frontendPath << std::endl;

    // ----------------------------------------------------------------------
    // Handle CORS preflight requests globally
    // ----------------------------------------------------------------------
    CROW_ROUTE(app, "/api/<path>")
    .methods(crow::HTTPMethod::Options)([](const std::string& /*path*/) {
        crow::response res(200);
        res.add_header("Access-Control-Allow-Origin", "*");
        res.add_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.add_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        return res;
    });

    // ----------------------------------------------------------------------
    // Health Check & Utilities for Cloud Hosting (Render, Railway, etc.)
    // ----------------------------------------------------------------------
    CROW_ROUTE(app, "/healthz")([]() {
        crow::json::wvalue res;
        res["status"] = "healthy";
        res["service"] = "LifeLink Blood Donor Finder";
        res["version"] = "1.0.0";
        return jsonResponse(200, res);
    });

    CROW_ROUTE(app, "/favicon.ico")([frontendPath]() {
        return serveFile(frontendPath + "/images/logo.svg");
    });

    // ----------------------------------------------------------------------
    // Static Frontend Routes
    // ----------------------------------------------------------------------
    CROW_ROUTE(app, "/")([frontendPath]() {
        return serveFile(frontendPath + "/index.html");
    });

    CROW_ROUTE(app, "/index.html")([frontendPath]() {
        return serveFile(frontendPath + "/index.html");
    });

    CROW_ROUTE(app, "/donors.html")([frontendPath]() {
        return serveFile(frontendPath + "/donors.html");
    });

    CROW_ROUTE(app, "/request.html")([frontendPath]() {
        return serveFile(frontendPath + "/request.html");
    });

    CROW_ROUTE(app, "/dashboard.html")([frontendPath]() {
        return serveFile(frontendPath + "/dashboard.html");
    });

    CROW_ROUTE(app, "/login.html")([frontendPath]() {
        return serveFile(frontendPath + "/login.html");
    });

    CROW_ROUTE(app, "/register.html")([frontendPath]() {
        return serveFile(frontendPath + "/register.html");
    });

    CROW_ROUTE(app, "/css/<string>")([frontendPath](const std::string& filename) {
        return serveFile(frontendPath + "/css/" + filename);
    });

    CROW_ROUTE(app, "/js/<string>")([frontendPath](const std::string& filename) {
        return serveFile(frontendPath + "/js/" + filename);
    });

    CROW_ROUTE(app, "/images/<string>")([frontendPath](const std::string& filename) {
        return serveFile(frontendPath + "/images/" + filename);
    });

    // ----------------------------------------------------------------------
    // API: System Statistics & Overview
    // ----------------------------------------------------------------------
    CROW_ROUTE(app, "/api/stats").methods(crow::HTTPMethod::Get)([&db]() {
        Statistics s = db.getStatistics();
        crow::json::wvalue res;
        res["success"] = true;
        res["total_donors"] = s.total_donors;
        res["available_donors"] = s.available_donors;
        res["total_requests"] = s.total_requests;
        res["pending_requests"] = s.pending_requests;
        res["fulfilled_requests"] = s.fulfilled_requests;
        res["total_units_needed"] = s.total_units_needed;

        std::vector<crow::json::wvalue> bgList;
        for (const auto& bg : s.blood_groups) {
            crow::json::wvalue bgObj;
            bgObj["blood_group"] = bg.blood_group;
            bgObj["count"] = bg.count;
            bgList.push_back(std::move(bgObj));
        }
        res["blood_groups"] = std::move(bgList);

        // Recent 5 donors
        auto recentDonors = db.getDonors("", "", 0);
        std::vector<crow::json::wvalue> dList;
        int dCount = 0;
        for (const auto& d : recentDonors) {
            if (++dCount > 5) break;
            crow::json::wvalue dObj;
            dObj["id"] = d.id;
            dObj["name"] = d.name;
            dObj["blood_group"] = d.blood_group;
            dObj["city"] = d.city;
            dObj["phone"] = d.phone;
            dObj["is_available"] = d.is_available;
            dObj["created_at"] = d.created_at;
            dList.push_back(std::move(dObj));
        }
        res["recent_donors"] = std::move(dList);

        // Recent 5 requests
        auto recentRequests = db.getBloodRequests("", "", "");
        std::vector<crow::json::wvalue> rList;
        int rCount = 0;
        for (const auto& r : recentRequests) {
            if (++rCount > 5) break;
            crow::json::wvalue rObj;
            rObj["id"] = r.id;
            rObj["patient_name"] = r.patient_name;
            rObj["blood_group"] = r.blood_group;
            rObj["units_needed"] = r.units_needed;
            rObj["hospital_name"] = r.hospital_name;
            rObj["city"] = r.city;
            rObj["urgency"] = r.urgency;
            rObj["status"] = r.status;
            rObj["created_at"] = r.created_at;
            rList.push_back(std::move(rObj));
        }
        res["recent_requests"] = std::move(rList);

        return jsonResponse(200, res);
    });

    // ----------------------------------------------------------------------
    // API: Donors CRUD
    // ----------------------------------------------------------------------
    // GET /api/donors?blood_group=...&city=...&available_only=1
    CROW_ROUTE(app, "/api/donors").methods(crow::HTTPMethod::Get)([&db](const crow::request& req) {
        std::string bg = req.url_params.get("blood_group") ? req.url_params.get("blood_group") : "";
        std::string city = req.url_params.get("city") ? req.url_params.get("city") : "";
        int avail = req.url_params.get("available_only") ? std::stoi(req.url_params.get("available_only")) : 0;

        auto donors = db.getDonors(bg, city, avail);
        crow::json::wvalue res;
        res["success"] = true;
        res["count"] = static_cast<int>(donors.size());

        std::vector<crow::json::wvalue> list;
        for (const auto& d : donors) {
            crow::json::wvalue obj;
            obj["id"] = d.id;
            obj["user_id"] = d.user_id;
            obj["name"] = d.name;
            obj["age"] = d.age;
            obj["gender"] = d.gender;
            obj["blood_group"] = d.blood_group;
            obj["phone"] = d.phone;
            obj["email"] = d.email;
            obj["city"] = d.city;
            obj["state"] = d.state;
            obj["address"] = d.address;
            obj["is_available"] = d.is_available;
            obj["last_donation_date"] = d.last_donation_date;
            obj["created_at"] = d.created_at;
            list.push_back(std::move(obj));
        }
        res["donors"] = std::move(list);
        return jsonResponse(200, res);
    });

    // GET /api/donors/<int>
    CROW_ROUTE(app, "/api/donors/<int>").methods(crow::HTTPMethod::Get)([&db](int id) {
        auto dOpt = db.getDonorById(id);
        if (!dOpt.has_value()) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Donor not found";
            return jsonResponse(404, err);
        }
        const auto& d = dOpt.value();
        crow::json::wvalue res;
        res["success"] = true;
        res["donor"]["id"] = d.id;
        res["donor"]["name"] = d.name;
        res["donor"]["age"] = d.age;
        res["donor"]["gender"] = d.gender;
        res["donor"]["blood_group"] = d.blood_group;
        res["donor"]["phone"] = d.phone;
        res["donor"]["email"] = d.email;
        res["donor"]["city"] = d.city;
        res["donor"]["state"] = d.state;
        res["donor"]["address"] = d.address;
        res["donor"]["is_available"] = d.is_available;
        res["donor"]["last_donation_date"] = d.last_donation_date;
        res["donor"]["created_at"] = d.created_at;
        return jsonResponse(200, res);
    });

    // POST /api/donors (Register a new donor)
    CROW_ROUTE(app, "/api/donors").methods(crow::HTTPMethod::Post)([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Invalid JSON payload";
            return jsonResponse(400, err);
        }

        // Validate required fields
        if (!body.has("name") || !body.has("blood_group") || !body.has("phone") || !body.has("city")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Missing required fields: name, blood_group, phone, and city are mandatory.";
            return jsonResponse(400, err);
        }

        Donor d;
        d.name = std::string(body["name"].s());
        d.age = body.has("age") ? body["age"].i() : 25;
        d.gender = body.has("gender") ? std::string(body["gender"].s()) : "Unspecified";
        d.blood_group = std::string(body["blood_group"].s());
        d.phone = std::string(body["phone"].s());
        d.email = body.has("email") ? std::string(body["email"].s()) : "";
        d.city = std::string(body["city"].s());
        d.state = body.has("state") ? std::string(body["state"].s()) : "";
        d.address = body.has("address") ? std::string(body["address"].s()) : "";
        d.is_available = body.has("is_available") ? body["is_available"].i() : 1;
        d.last_donation_date = body.has("last_donation_date") ? std::string(body["last_donation_date"].s()) : "";

        if (body.has("user_id")) {
            d.user_id = body["user_id"].i();
        }

        // Basic validation check
        if (d.age < 18 || d.age > 65) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Donor age must be between 18 and 65 years according to medical guidelines.";
            return jsonResponse(400, err);
        }

        int newId = 0;
        if (db.addDonor(d, &newId)) {
            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Donor registered successfully!";
            res["id"] = newId;
            return jsonResponse(201, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to save donor record in database.";
            return jsonResponse(500, err);
        }
    });

    // PUT /api/donors/<int>/availability
    CROW_ROUTE(app, "/api/donors/<int>/availability").methods(crow::HTTPMethod::Put)([&db](const crow::request& req, int id) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("is_available")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Field 'is_available' (1 or 0) required.";
            return jsonResponse(400, err);
        }

        int status = body["is_available"].i();
        if (db.updateDonorAvailability(id, status)) {
            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Donor availability updated successfully!";
            res["is_available"] = status;
            return jsonResponse(200, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to update donor availability.";
            return jsonResponse(404, err);
        }
    });

    // DELETE /api/donors/<int>
    CROW_ROUTE(app, "/api/donors/<int>").methods(crow::HTTPMethod::Delete)([&db](int id) {
        if (db.deleteDonor(id)) {
            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Donor record deleted successfully.";
            return jsonResponse(200, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to delete donor or donor not found.";
            return jsonResponse(404, err);
        }
    });

    // ----------------------------------------------------------------------
    // API: Blood Requests CRUD
    // ----------------------------------------------------------------------
    // GET /api/requests?status=...&blood_group=...&city=...
    CROW_ROUTE(app, "/api/requests").methods(crow::HTTPMethod::Get)([&db](const crow::request& req) {
        std::string status = req.url_params.get("status") ? req.url_params.get("status") : "";
        std::string bg = req.url_params.get("blood_group") ? req.url_params.get("blood_group") : "";
        std::string city = req.url_params.get("city") ? req.url_params.get("city") : "";

        auto reqs = db.getBloodRequests(status, bg, city);
        crow::json::wvalue res;
        res["success"] = true;
        res["count"] = static_cast<int>(reqs.size());

        std::vector<crow::json::wvalue> list;
        for (const auto& r : reqs) {
            crow::json::wvalue obj;
            obj["id"] = r.id;
            obj["patient_name"] = r.patient_name;
            obj["blood_group"] = r.blood_group;
            obj["units_needed"] = r.units_needed;
            obj["hospital_name"] = r.hospital_name;
            obj["city"] = r.city;
            obj["contact_person"] = r.contact_person;
            obj["contact_phone"] = r.contact_phone;
            obj["urgency"] = r.urgency;
            obj["status"] = r.status;
            obj["notes"] = r.notes;
            obj["created_at"] = r.created_at;
            list.push_back(std::move(obj));
        }
        res["requests"] = std::move(list);
        return jsonResponse(200, res);
    });

    // GET /api/requests/<int>
    CROW_ROUTE(app, "/api/requests/<int>").methods(crow::HTTPMethod::Get)([&db](int id) {
        auto rOpt = db.getBloodRequestById(id);
        if (!rOpt.has_value()) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Blood request not found";
            return jsonResponse(404, err);
        }
        const auto& r = rOpt.value();
        crow::json::wvalue res;
        res["success"] = true;
        res["request"]["id"] = r.id;
        res["request"]["patient_name"] = r.patient_name;
        res["request"]["blood_group"] = r.blood_group;
        res["request"]["units_needed"] = r.units_needed;
        res["request"]["hospital_name"] = r.hospital_name;
        res["request"]["city"] = r.city;
        res["request"]["contact_person"] = r.contact_person;
        res["request"]["contact_phone"] = r.contact_phone;
        res["request"]["urgency"] = r.urgency;
        res["request"]["status"] = r.status;
        res["request"]["notes"] = r.notes;
        res["request"]["created_at"] = r.created_at;
        return jsonResponse(200, res);
    });

    // POST /api/requests (Submit an Emergency Blood Request)
    CROW_ROUTE(app, "/api/requests").methods(crow::HTTPMethod::Post)([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Invalid JSON payload";
            return jsonResponse(400, err);
        }

        if (!body.has("patient_name") || !body.has("blood_group") || !body.has("hospital_name") || !body.has("contact_phone") || !body.has("city")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Missing required fields: patient_name, blood_group, hospital_name, city, and contact_phone are mandatory.";
            return jsonResponse(400, err);
        }

        BloodRequest r;
        r.patient_name = std::string(body["patient_name"].s());
        r.blood_group = std::string(body["blood_group"].s());
        r.units_needed = body.has("units_needed") ? body["units_needed"].i() : 1;
        r.hospital_name = std::string(body["hospital_name"].s());
        r.city = std::string(body["city"].s());
        r.contact_person = body.has("contact_person") ? std::string(body["contact_person"].s()) : r.patient_name;
        r.contact_phone = std::string(body["contact_phone"].s());
        r.urgency = body.has("urgency") ? std::string(body["urgency"].s()) : "Urgent";
        r.status = "Pending";
        r.notes = body.has("notes") ? std::string(body["notes"].s()) : "";

        if (r.units_needed <= 0) r.units_needed = 1;

        int newId = 0;
        if (db.addBloodRequest(r, &newId)) {
            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Emergency blood request submitted successfully!";
            res["id"] = newId;
            return jsonResponse(201, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to save blood request in database.";
            return jsonResponse(500, err);
        }
    });

    // PUT /api/requests/<int>/status (Mark as Fulfilled or Cancelled)
    CROW_ROUTE(app, "/api/requests/<int>/status").methods(crow::HTTPMethod::Put)([&db](const crow::request& req, int id) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("status")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Field 'status' is required ('Pending', 'Fulfilled', 'Cancelled').";
            return jsonResponse(400, err);
        }

        std::string newStatus = std::string(body["status"].s());
        if (db.updateBloodRequestStatus(id, newStatus)) {
            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Blood request status updated to: " + newStatus;
            res["status"] = newStatus;
            return jsonResponse(200, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to update blood request status.";
            return jsonResponse(404, err);
        }
    });

    // DELETE /api/requests/<int>
    CROW_ROUTE(app, "/api/requests/<int>").methods(crow::HTTPMethod::Delete)([&db](int id) {
        if (db.deleteBloodRequest(id)) {
            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "Blood request deleted successfully.";
            return jsonResponse(200, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to delete blood request or request not found.";
            return jsonResponse(404, err);
        }
    });

    // ----------------------------------------------------------------------
    // API: Authentication (Register & Login)
    // ----------------------------------------------------------------------
    CROW_ROUTE(app, "/api/auth/register").methods(crow::HTTPMethod::Post)([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("name") || !body.has("email") || !body.has("password") || !body.has("phone")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Missing fields: name, email, password, and phone are required.";
            return jsonResponse(400, err);
        }

        std::string email = std::string(body["email"].s());
        if (db.getUserByEmail(email).has_value()) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "An account with this email address already exists. Please log in.";
            return jsonResponse(409, err);
        }

        User u;
        u.name = std::string(body["name"].s());
        u.email = email;
        u.password = std::string(body["password"].s());
        u.phone = std::string(body["phone"].s());
        u.role = body.has("role") ? std::string(body["role"].s()) : "donor";

        int newUserId = 0;
        if (db.registerUser(u, &newUserId)) {
            // Also optionally register as a donor if blood_group is provided!
            if (body.has("blood_group") && body.has("city")) {
                Donor d;
                d.user_id = newUserId;
                d.name = u.name;
                d.age = body.has("age") ? body["age"].i() : 25;
                d.gender = body.has("gender") ? std::string(body["gender"].s()) : "Unspecified";
                d.blood_group = std::string(body["blood_group"].s());
                d.phone = u.phone;
                d.email = u.email;
                d.city = std::string(body["city"].s());
                d.state = body.has("state") ? std::string(body["state"].s()) : "";
                d.address = body.has("address") ? std::string(body["address"].s()) : "";
                d.is_available = 1;
                db.addDonor(d);
            }

            crow::json::wvalue res;
            res["success"] = true;
            res["message"] = "User registered successfully!";
            res["user"]["id"] = newUserId;
            res["user"]["name"] = u.name;
            res["user"]["email"] = u.email;
            res["user"]["phone"] = u.phone;
            res["user"]["role"] = u.role;
            return jsonResponse(201, res);
        } else {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Failed to create user account.";
            return jsonResponse(500, err);
        }
    });

    CROW_ROUTE(app, "/api/auth/login").methods(crow::HTTPMethod::Post)([&db](const crow::request& req) {
        auto body = crow::json::load(req.body);
        if (!body || !body.has("email") || !body.has("password")) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Email and password are required.";
            return jsonResponse(400, err);
        }

        std::string email = std::string(body["email"].s());
        std::string password = std::string(body["password"].s());

        auto userOpt = db.authenticateUser(email, password);
        if (!userOpt.has_value()) {
            crow::json::wvalue err;
            err["success"] = false;
            err["message"] = "Invalid email or password. Please verify your credentials.";
            return jsonResponse(401, err);
        }

        const auto& u = userOpt.value();
        crow::json::wvalue res;
        res["success"] = true;
        res["message"] = "Login successful!";
        res["user"]["id"] = u.id;
        res["user"]["name"] = u.name;
        res["user"]["email"] = u.email;
        res["user"]["phone"] = u.phone;
        res["user"]["role"] = u.role;
        return jsonResponse(200, res);
    });

    std::cout << "[INFO] Starting LifeLink server on http://0.0.0.0:" << port << std::endl;
    std::cout << "[INFO] Listening on all network interfaces (0.0.0.0) with PORT=" << port << std::endl;
    std::cout << "[INFO] Press Ctrl+C in terminal to stop server." << std::endl;

    app.bindaddr("0.0.0.0").port(port).multithreaded().run();
    return 0;
}
