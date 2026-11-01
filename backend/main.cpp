// LifeLink - Blood Donor Finder : C++ REST backend (Crow + SQLite)
#include "crow_all.h"
#include <sqlite3.h>
#include <fstream>
#include <sstream>
#include <regex>
#include <mutex>
#include <set>
#include <string>
#include <vector>

static sqlite3* db = nullptr;
static std::mutex dbMutex;

// ---------- helpers ----------
static const std::set<std::string> BLOOD_GROUPS = {"A+","A-","B+","B-","AB+","AB-","O+","O-"};

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    if (!f) return "";
    std::stringstream ss; ss << f.rdbuf(); return ss.str();
}

static bool execSql(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        CROW_LOG_ERROR << "SQL error: " << (err ? err : "?");
        sqlite3_free(err);
        return false;
    }
    return true;
}

static crow::response jsonError(int code, const std::string& msg) {
    crow::json::wvalue e; e["error"] = msg;
    crow::response r(code, e);
    r.set_header("Content-Type", "application/json");
    return r;
}

static crow::response jsonOk(crow::json::wvalue v, int code = 200) {
    crow::response r(code, v);
    r.set_header("Content-Type", "application/json");
    return r;
}

static std::string col(sqlite3_stmt* s, int i) {
    auto t = sqlite3_column_text(s, i);
    return t ? reinterpret_cast<const char*>(t) : "";
}

static std::string trim(const std::string& s) {
    auto a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    auto b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

static std::string getStr(const crow::json::rvalue& j, const char* k) {
    if (!j.has(k)) return "";
    if (j[k].t() == crow::json::type::String) return trim(std::string(j[k].s()));
    if (j[k].t() == crow::json::type::Number) return std::to_string(j[k].i());
    return "";
}
static long getInt(const crow::json::rvalue& j, const char* k, long def = -1) {
    if (!j.has(k)) return def;
    if (j[k].t() == crow::json::type::Number) return j[k].i();
    if (j[k].t() == crow::json::type::String) { try { return std::stol(std::string(j[k].s())); } catch (...) {} }
    if (j[k].t() == crow::json::type::True) return 1;
    if (j[k].t() == crow::json::type::False) return 0;
    return def;
}

static bool validPhone(const std::string& p) { return std::regex_match(p, std::regex("^[6-9][0-9]{9}$")); }
static bool validEmail(const std::string& e) { return std::regex_match(e, std::regex("^[^@\\s]+@[^@\\s]+\\.[^@\\s]+$")); }
static bool validName(const std::string& n) { return n.size() >= 2 && n.size() <= 100 && std::regex_match(n, std::regex("^[A-Za-z .'-]+$")); }
static bool validDate(const std::string& d) { return std::regex_match(d, std::regex("^\\d{4}-\\d{2}-\\d{2}$")); }

// Privacy: mask phone like 98XXXXXX10
static std::string maskPhone(const std::string& p) {
    if (p.size() < 4) return "XXXX";
    return p.substr(0, 2) + std::string(p.size() - 4, 'X') + p.substr(p.size() - 2);
}
static std::string maskEmail(const std::string& e) {
    auto at = e.find('@');
    if (at == std::string::npos || at < 1) return "hidden";
    return e.substr(0, 1) + "***" + e.substr(at);
}

static void bindText(sqlite3_stmt* s, int i, const std::string& v) {
    sqlite3_bind_text(s, i, v.c_str(), -1, SQLITE_TRANSIENT);
}

// ---------- donor validation ----------
struct Donor {
    std::string name, gender, phone, email, blood, city, state;
    long age = 0, available = 1, consent = 0;
};

static std::string validateDonor(const crow::json::rvalue& j, Donor& d) {
    d.name = getStr(j, "full_name"); d.gender = getStr(j, "gender");
    d.phone = getStr(j, "mobile"); d.email = getStr(j, "email");
    d.blood = getStr(j, "blood_group"); d.city = getStr(j, "city");
    d.state = getStr(j, "state"); d.age = getInt(j, "age");
    d.available = getInt(j, "available", 1); d.consent = getInt(j, "consent_share_contact", 0);
    if (!validName(d.name)) return "Please enter a valid full name (letters only).";
    if (d.age < 18 || d.age > 65) return "Donor age must be between 18 and 65.";
    if (d.gender != "Male" && d.gender != "Female" && d.gender != "Other") return "Please select a gender.";
    if (!validPhone(d.phone)) return "Mobile number must be a valid 10-digit Indian number.";
    if (!validEmail(d.email) || d.email.size() > 120) return "Please enter a valid email address.";
    if (!BLOOD_GROUPS.count(d.blood)) return "Please select a valid blood group.";
    if (d.city.size() < 2 || d.city.size() > 60) return "Please enter a valid city.";
    if (d.state.size() < 2 || d.state.size() > 60) return "Please enter a valid state.";
    if (d.available != 0 && d.available != 1) return "Invalid availability status.";
    return "";
}

struct Req {
    std::string patient, blood, hospital, city, contact, level, date;
    long units = 0;
};

static std::string validateRequest(const crow::json::rvalue& j, Req& r) {
    r.patient = getStr(j, "patient_name"); r.blood = getStr(j, "blood_group");
    r.hospital = getStr(j, "hospital"); r.city = getStr(j, "city");
    r.contact = getStr(j, "contact"); r.level = getStr(j, "emergency_level");
    r.date = getStr(j, "required_date"); r.units = getInt(j, "units");
    if (!validName(r.patient)) return "Please enter a valid patient name.";
    if (!BLOOD_GROUPS.count(r.blood)) return "Please select a valid blood group.";
    if (r.hospital.size() < 3 || r.hospital.size() > 120) return "Please enter the hospital name.";
    if (r.city.size() < 2 || r.city.size() > 60) return "Please enter a valid city.";
    if (!validPhone(r.contact)) return "Contact must be a valid 10-digit Indian number.";
    if (r.units < 1 || r.units > 10) return "Units required must be between 1 and 10.";
    if (r.level != "Critical" && r.level != "High" && r.level != "Normal") return "Please select an emergency level.";
    if (!validDate(r.date)) return "Please choose the required date.";
    return "";
}

static crow::json::wvalue donorPublic(sqlite3_stmt* s) {
    // columns: id, full_name, age, gender, mobile, email, blood_group, city, state, available, consent
    crow::json::wvalue d;
    bool consent = sqlite3_column_int(s, 10) == 1;
    d["id"] = sqlite3_column_int(s, 0);
    d["full_name"] = col(s, 1);
    d["age"] = sqlite3_column_int(s, 2);
    d["gender"] = col(s, 3);
    d["blood_group"] = col(s, 6);
    d["city"] = col(s, 7);
    d["state"] = col(s, 8);
    d["available"] = sqlite3_column_int(s, 9) == 1;
    d["consent_share_contact"] = consent;
    d["mobile_masked"] = maskPhone(col(s, 4));
    d["email_masked"] = maskEmail(col(s, 5));
    return d;
}

static crow::json::wvalue requestRow(sqlite3_stmt* s) {
    crow::json::wvalue r;
    r["id"] = sqlite3_column_int(s, 0);
    r["patient_name"] = col(s, 1);
    r["blood_group"] = col(s, 2);
    r["hospital"] = col(s, 3);
    r["city"] = col(s, 4);
    r["contact"] = maskPhone(col(s, 5));
    r["units"] = sqlite3_column_int(s, 6);
    r["emergency_level"] = col(s, 7);
    r["required_date"] = col(s, 8);
    r["status"] = col(s, 9);
    r["created_at"] = col(s, 10);
    return r;
}

static const char* DONOR_COLS = "id, full_name, age, gender, mobile, email, blood_group, city, state, available, consent_share_contact";
static const char* REQ_COLS = "id, patient_name, blood_group, hospital, city, contact, units, emergency_level, required_date, status, created_at";

int main(int argc, char** argv) {
    std::string root = argc > 1 ? argv[1] : "..";   // project root (contains frontend/ and database/)
    std::string dbPath = root + "/database/lifelink.db";
    bool fresh = !std::ifstream(dbPath).good();

    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Cannot open database at " << dbPath << "\n"; return 1;
    }
    execSql("PRAGMA foreign_keys = ON;");
    std::string schema = readFile(root + "/database/schema.sql");
    if (schema.empty()) { std::cerr << "schema.sql not found. Run from backend/build with ../.. or pass project root.\n"; return 1; }
    execSql(schema);
    if (fresh) { execSql(readFile(root + "/database/seed.sql")); std::cout << "Database created with sample data.\n"; }

    crow::SimpleApp app;

    // ---------- static frontend ----------
    auto serveFile = [root](const std::string& rel) {
        std::string path = root + "/frontend/" + rel;
        if (rel.find("..") != std::string::npos) return crow::response(403);
        std::string body = readFile(path);
        if (body.empty()) return crow::response(404, "Not found");
        crow::response r(body);
        auto ext = rel.substr(rel.find_last_of('.') + 1);
        std::string type = ext == "html" ? "text/html; charset=utf-8" : ext == "css" ? "text/css" :
                           ext == "js" ? "application/javascript" : ext == "svg" ? "image/svg+xml" : "text/plain";
        r.set_header("Content-Type", type);
        return r;
    };

    // ---------- DONORS ----------
    // Create
    CROW_ROUTE(app, "/api/donors").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto j = crow::json::load(req.body);
        if (!j) return jsonError(400, "Invalid JSON body.");
        Donor d; auto err = validateDonor(j, d);
        if (!err.empty()) return jsonError(422, err);
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "INSERT INTO donors(full_name, age, gender, mobile, email, blood_group, city, state, available, consent_share_contact) VALUES(?,?,?,?,?,?,?,?,?,?)", -1, &s, nullptr);
        bindText(s,1,d.name); sqlite3_bind_int(s,2,d.age); bindText(s,3,d.gender); bindText(s,4,d.phone);
        bindText(s,5,d.email); bindText(s,6,d.blood); bindText(s,7,d.city); bindText(s,8,d.state);
        sqlite3_bind_int(s,9,d.available); sqlite3_bind_int(s,10,d.consent);
        int rc = sqlite3_step(s); sqlite3_finalize(s);
        if (rc == SQLITE_CONSTRAINT) return jsonError(409, "A donor with this mobile number is already registered.");
        if (rc != SQLITE_DONE) return jsonError(500, "Could not save donor.");
        crow::json::wvalue out; out["id"] = (int)sqlite3_last_insert_rowid(db); out["message"] = "Thank you for registering as a donor!";
        return jsonOk(std::move(out), 201);
    });

    // Read / search  (?blood_group=A%2B&city=Delhi&available=1)
    CROW_ROUTE(app, "/api/donors").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        std::string bg = req.url_params.get("blood_group") ? req.url_params.get("blood_group") : "";
        std::string city = req.url_params.get("city") ? trim(req.url_params.get("city")) : "";
        std::string avail = req.url_params.get("available") ? req.url_params.get("available") : "";
        if (!bg.empty() && !BLOOD_GROUPS.count(bg)) return jsonError(422, "Invalid blood group.");
        std::string sql = std::string("SELECT ") + DONOR_COLS + " FROM donors WHERE 1=1";
        if (!bg.empty()) sql += " AND blood_group = ?";
        if (!city.empty()) sql += " AND LOWER(city) LIKE LOWER(?)";
        if (avail == "1") sql += " AND available = 1";
        sql += " ORDER BY available DESC, full_name";
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s; sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr);
        int i = 1;
        if (!bg.empty()) bindText(s, i++, bg);
        if (!city.empty()) bindText(s, i++, "%" + city + "%");
        std::vector<crow::json::wvalue> list;
        while (sqlite3_step(s) == SQLITE_ROW) list.push_back(donorPublic(s));
        sqlite3_finalize(s);
        crow::json::wvalue out; out["count"] = (int)list.size(); out["donors"] = std::move(list);
        return jsonOk(std::move(out));
    });

    // Reveal contact (only with donor consent; requester must identify themselves)
    CROW_ROUTE(app, "/api/donors/<int>/contact").methods(crow::HTTPMethod::POST)([](const crow::request& req, int id) {
        auto j = crow::json::load(req.body);
        if (!j) return jsonError(400, "Invalid JSON body.");
        std::string rname = getStr(j, "requester_name"), rphone = getStr(j, "requester_phone");
        if (!validName(rname) || !validPhone(rphone)) return jsonError(422, "Please enter your name and a valid 10-digit mobile number.");
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "SELECT full_name, mobile, email, consent_share_contact FROM donors WHERE id=?", -1, &s, nullptr);
        sqlite3_bind_int(s, 1, id);
        if (sqlite3_step(s) != SQLITE_ROW) { sqlite3_finalize(s); return jsonError(404, "Donor not found."); }
        std::string name = col(s,0), mobile = col(s,1), email = col(s,2); int consent = sqlite3_column_int(s,3);
        sqlite3_finalize(s);
        if (!consent) return jsonError(403, "This donor has not agreed to share contact details publicly.");
        sqlite3_prepare_v2(db, "INSERT INTO contact_log(donor_id, requester_name, requester_phone) VALUES(?,?,?)", -1, &s, nullptr);
        sqlite3_bind_int(s,1,id); bindText(s,2,rname); bindText(s,3,rphone); sqlite3_step(s); sqlite3_finalize(s);
        crow::json::wvalue out; out["full_name"] = name; out["mobile"] = mobile; out["email"] = email;
        return jsonOk(std::move(out));
    });

    // Update (donor proves ownership with registered mobile)
    CROW_ROUTE(app, "/api/donors/<int>").methods(crow::HTTPMethod::PUT)([](const crow::request& req, int id) {
        auto j = crow::json::load(req.body);
        if (!j) return jsonError(400, "Invalid JSON body.");
        Donor d; auto err = validateDonor(j, d);
        if (!err.empty()) return jsonError(422, err);
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "UPDATE donors SET full_name=?, age=?, gender=?, email=?, blood_group=?, city=?, state=?, available=?, consent_share_contact=? WHERE id=? AND mobile=?", -1, &s, nullptr);
        bindText(s,1,d.name); sqlite3_bind_int(s,2,d.age); bindText(s,3,d.gender); bindText(s,4,d.email);
        bindText(s,5,d.blood); bindText(s,6,d.city); bindText(s,7,d.state); sqlite3_bind_int(s,8,d.available);
        sqlite3_bind_int(s,9,d.consent); sqlite3_bind_int(s,10,id); bindText(s,11,d.phone);
        sqlite3_step(s); sqlite3_finalize(s);
        if (sqlite3_changes(db) == 0) return jsonError(404, "Donor not found or mobile number does not match.");
        crow::json::wvalue out; out["message"] = "Donor details updated.";
        return jsonOk(std::move(out));
    });

    // Delete (requires registered mobile: /api/donors/5?mobile=98...)
    CROW_ROUTE(app, "/api/donors/<int>").methods(crow::HTTPMethod::DELETE)([](const crow::request& req, int id) {
        std::string mobile = req.url_params.get("mobile") ? req.url_params.get("mobile") : "";
        if (!validPhone(mobile)) return jsonError(422, "Registered mobile number is required.");
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "DELETE FROM donors WHERE id=? AND mobile=?", -1, &s, nullptr);
        sqlite3_bind_int(s,1,id); bindText(s,2,mobile); sqlite3_step(s); sqlite3_finalize(s);
        if (sqlite3_changes(db) == 0) return jsonError(404, "Donor not found or mobile number does not match.");
        crow::json::wvalue out; out["message"] = "Donor record deleted.";
        return jsonOk(std::move(out));
    });

    // Look up own donor profile by mobile (for edit/delete)
    CROW_ROUTE(app, "/api/donors/me").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        std::string mobile = req.url_params.get("mobile") ? req.url_params.get("mobile") : "";
        if (!validPhone(mobile)) return jsonError(422, "Enter a valid 10-digit mobile number.");
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, (std::string("SELECT ") + DONOR_COLS + " FROM donors WHERE mobile=?").c_str(), -1, &s, nullptr);
        bindText(s,1,mobile);
        if (sqlite3_step(s) != SQLITE_ROW) { sqlite3_finalize(s); return jsonError(404, "No donor registered with this mobile number."); }
        crow::json::wvalue d;
        d["id"] = sqlite3_column_int(s,0); d["full_name"] = col(s,1); d["age"] = sqlite3_column_int(s,2);
        d["gender"] = col(s,3); d["mobile"] = col(s,4); d["email"] = col(s,5); d["blood_group"] = col(s,6);
        d["city"] = col(s,7); d["state"] = col(s,8); d["available"] = sqlite3_column_int(s,9);
        d["consent_share_contact"] = sqlite3_column_int(s,10);
        sqlite3_finalize(s);
        return jsonOk(std::move(d));
    });

    // ---------- BLOOD REQUESTS ----------
    CROW_ROUTE(app, "/api/requests").methods(crow::HTTPMethod::POST)([](const crow::request& req) {
        auto j = crow::json::load(req.body);
        if (!j) return jsonError(400, "Invalid JSON body.");
        Req r; auto err = validateRequest(j, r);
        if (!err.empty()) return jsonError(422, err);
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "INSERT INTO blood_requests(patient_name, blood_group, hospital, city, contact, units, emergency_level, required_date) VALUES(?,?,?,?,?,?,?,?)", -1, &s, nullptr);
        bindText(s,1,r.patient); bindText(s,2,r.blood); bindText(s,3,r.hospital); bindText(s,4,r.city);
        bindText(s,5,r.contact); sqlite3_bind_int(s,6,r.units); bindText(s,7,r.level); bindText(s,8,r.date);
        int rc = sqlite3_step(s); sqlite3_finalize(s);
        if (rc != SQLITE_DONE) return jsonError(500, "Could not save request.");
        crow::json::wvalue out; out["id"] = (int)sqlite3_last_insert_rowid(db);
        out["message"] = "Emergency request submitted. Matching donors can now find it.";
        return jsonOk(std::move(out), 201);
    });

    // List requests: ?contact=98xxxxxxxx shows that user's requests; ?status=Pending
    CROW_ROUTE(app, "/api/requests").methods(crow::HTTPMethod::GET)([](const crow::request& req) {
        std::string contact = req.url_params.get("contact") ? req.url_params.get("contact") : "";
        std::string status = req.url_params.get("status") ? req.url_params.get("status") : "";
        if (!contact.empty() && !validPhone(contact)) return jsonError(422, "Enter a valid 10-digit mobile number.");
        std::string sql = std::string("SELECT ") + REQ_COLS + " FROM blood_requests WHERE 1=1";
        if (!contact.empty()) sql += " AND contact = ?";
        if (status == "Pending" || status == "Fulfilled") sql += " AND status = ?";
        sql += " ORDER BY CASE emergency_level WHEN 'Critical' THEN 0 WHEN 'High' THEN 1 ELSE 2 END, required_date";
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s; sqlite3_prepare_v2(db, sql.c_str(), -1, &s, nullptr);
        int i = 1;
        if (!contact.empty()) bindText(s, i++, contact);
        if (status == "Pending" || status == "Fulfilled") bindText(s, i++, status);
        std::vector<crow::json::wvalue> list;
        while (sqlite3_step(s) == SQLITE_ROW) list.push_back(requestRow(s));
        sqlite3_finalize(s);
        crow::json::wvalue out; out["count"] = (int)list.size(); out["requests"] = std::move(list);
        return jsonOk(std::move(out));
    });

    // Update status (owner only: body {contact, status})
    CROW_ROUTE(app, "/api/requests/<int>").methods(crow::HTTPMethod::PUT)([](const crow::request& req, int id) {
        auto j = crow::json::load(req.body);
        if (!j) return jsonError(400, "Invalid JSON body.");
        std::string contact = getStr(j, "contact"), status = getStr(j, "status");
        if (status != "Pending" && status != "Fulfilled") return jsonError(422, "Status must be Pending or Fulfilled.");
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "UPDATE blood_requests SET status=? WHERE id=? AND contact=?", -1, &s, nullptr);
        bindText(s,1,status); sqlite3_bind_int(s,2,id); bindText(s,3,contact); sqlite3_step(s); sqlite3_finalize(s);
        if (sqlite3_changes(db) == 0) return jsonError(404, "Request not found or contact number does not match.");
        crow::json::wvalue out; out["message"] = "Request marked as " + status + ".";
        return jsonOk(std::move(out));
    });

    CROW_ROUTE(app, "/api/requests/<int>").methods(crow::HTTPMethod::DELETE)([](const crow::request& req, int id) {
        std::string contact = req.url_params.get("contact") ? req.url_params.get("contact") : "";
        std::lock_guard<std::mutex> lk(dbMutex);
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "DELETE FROM blood_requests WHERE id=? AND contact=?", -1, &s, nullptr);
        sqlite3_bind_int(s,1,id); bindText(s,2,contact); sqlite3_step(s); sqlite3_finalize(s);
        if (sqlite3_changes(db) == 0) return jsonError(404, "Request not found or contact number does not match.");
        crow::json::wvalue out; out["message"] = "Request deleted.";
        return jsonOk(std::move(out));
    });

    // ---------- STATS ----------
    CROW_ROUTE(app, "/api/stats")([]() {
        std::lock_guard<std::mutex> lk(dbMutex);
        auto count = [](const char* sql) {
            sqlite3_stmt* s; sqlite3_prepare_v2(db, sql, -1, &s, nullptr);
            int n = sqlite3_step(s) == SQLITE_ROW ? sqlite3_column_int(s, 0) : 0;
            sqlite3_finalize(s); return n;
        };
        crow::json::wvalue o;
        o["total_donors"] = count("SELECT COUNT(*) FROM donors");
        o["available_donors"] = count("SELECT COUNT(*) FROM donors WHERE available=1");
        o["total_requests"] = count("SELECT COUNT(*) FROM blood_requests");
        o["pending_requests"] = count("SELECT COUNT(*) FROM blood_requests WHERE status='Pending'");
        o["fulfilled_requests"] = count("SELECT COUNT(*) FROM blood_requests WHERE status='Fulfilled'");
        o["cities"] = count("SELECT COUNT(DISTINCT LOWER(city)) FROM donors");
        std::vector<crow::json::wvalue> groups;
        sqlite3_stmt* s;
        sqlite3_prepare_v2(db, "SELECT blood_group, COUNT(*) FROM donors GROUP BY blood_group ORDER BY blood_group", -1, &s, nullptr);
        while (sqlite3_step(s) == SQLITE_ROW) { crow::json::wvalue g; g["group"] = col(s,0); g["count"] = sqlite3_column_int(s,1); groups.push_back(std::move(g)); }
        sqlite3_finalize(s);
        o["by_group"] = std::move(groups);
        return jsonOk(std::move(o));
    });

    CROW_ROUTE(app, "/")([serveFile]() { return serveFile("index.html"); });
    CROW_ROUTE(app, "/<path>")([serveFile](const std::string& p) { return serveFile(p); });

    int port = 18080;
    std::cout << "LifeLink running at http://localhost:" << port << "\n";
    app.port(port).multithreaded().run();
    sqlite3_close(db);
    return 0;
}
