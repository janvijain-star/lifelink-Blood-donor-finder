# LifeLink – Blood Donor Finder
**B.Tech C++ Internship Project**

[![C++17](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B17)
[![Framework](https://img.shields.io/badge/Framework-Crow%20C%2B%2B-red.svg)](https://crowcpp.org/)
[![Database](https://img.shields.io/badge/Database-SQLite3-003B57.svg)](https://www.sqlite.org/)
[![Frontend](https://img.shields.io/badge/Frontend-HTML5%20%7C%20CSS3%20%7C%20JS-crimson.svg)](https://developer.mozilla.org/)
[![License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

**LifeLink** is a full-stack, responsive web application built with **C++** as the backend language, **Crow** as the C++ micro web framework, **SQLite3** for persistent relational storage, and a modern, responsive **HTML5/CSS3/JavaScript** frontend with a professional red-and-white theme.

Unlike mobile apps or projects running on Node.js/Python, LifeLink uses pure, compiled **C++17** to achieve high performance, memory efficiency, and direct multi-threaded API serving for critical emergency healthcare workflows.

---

## 🌟 Key Features

### 1. Frontend (Responsive Web Application)
- **Theme:** Clean, modern medical red-and-white aesthetic with smooth micro-animations.
- **Responsive:** Full mobile, tablet, and desktop browser compatibility.
- **Homepage (`index.html`):** Hero section with quick search, live counters, 3-step process overview, urgent requests feed, and blood compatibility matrix.
- **Donor Directory (`donors.html`):** Filter by blood group and city in real-time, toggle between Grid Cards and Table views, instant "Call Donor" and "Copy Number" actions.
- **Emergency Requests (`request.html`):** Broadcast patient needs with hospital name, units needed, urgency tag (Critical / Urgent / Standard), and notes. Live emergency board with one-click "Mark as Fulfilled".
- **Dashboard (`dashboard.html`):** Analytics overview with KPI cards, 8-blood-group availability inventory, and tables for recent donors and emergency requests.
- **User Authentication (`login.html` & `register.html`):** Secure login, donor registration with medical validation (age 18–65, 10-digit phone, pledge), and quick-fill demo buttons for internship evaluation.

### 2. Backend & Database (C++ & SQLite)
- **C++ Backend:** Written in C++17 using the Crow web microframework.
- **SQLite Database:** Single-file relational database (`database/lifelink.db`) with parameterized queries to prevent SQL injection.
- **CRUD Operations:**
  - **Donors:** Create, Read (all/filtered by group & city), Update availability, Delete.
  - **Requests:** Submit emergency request, Read by status/group, Update status (Pending/Fulfilled), Delete.
  - **Analytics:** Dynamic calculation of available donors per blood group and pending unit counts.
  - **Authentication:** Password validation and session handling.
- **Dual Serving:** The C++ backend serves both REST API endpoints (`/api/...`) AND static frontend assets directly on port `8080`.

---

## 📁 Project Directory Structure

```text
happy-raman/
├── backend/
│   ├── include/
│   │   ├── crow_all.h        # Crow C++ header-only microframework (v1.3.5)
│   │   ├── asio.hpp          # Standalone Asio networking headers
│   │   ├── asio/...          # Asio implementation files
│   │   └── sqlite3.h         # SQLite3 C/C++ interface
│   ├── sqlite3.c             # SQLite3 engine (amalgamation source)
│   ├── sqlite3.o             # Precompiled SQLite object file
│   ├── db_manager.hpp        # Database Manager class declaration
│   ├── db_manager.cpp        # SQLite CRUD implementation & parameterized queries
│   ├── server.cpp            # Crow server, REST routes & static file handlers
│   ├── build.bat             # Windows compilation batch script
│   └── Makefile              # Cross-platform Makefile for GNU Make
├── database/
│   ├── schema.sql            # SQL schema script with sample initial data
│   └── lifelink.db           # SQLite database file (auto-generated on launch)
├── frontend/
│   ├── index.html            # Homepage with quick search & hero counters
│   ├── donors.html           # Donor search & directory (Grid / Table view)
│   ├── request.html          # Emergency blood request form & active board
│   ├── dashboard.html        # Analytics dashboard & blood inventory grid
│   ├── login.html            # User login with evaluation demo buttons
│   ├── register.html         # Donor registration with medical validation
│   ├── css/
│   │   └── styles.css        # Modern red & white design stylesheet
│   ├── js/
│   │   ├── api.js            # API client communicating with C++ backend
│   │   └── app.js            # Application state, UI rendering & form handlers
│   └── images/
│       └── logo.svg          # Custom LifeLink vector brand logo
├── run.bat                   # 1-Click launcher (compiles if needed & opens browser)
└── README.md                 # Project documentation & execution guide
```

---

## 🚀 Quick Start Guide (Windows)

### Method 1: The One-Click Launcher (Recommended)
Simply double-click or run from PowerShell / Command Prompt:
```cmd
.\run.bat
```
What this does automatically:
1. Detects modern GCC in PATH or `C:\Users\pc\w64devkit\bin`.
2. Checks if `backend\lifelink_server.exe` is built; if not, compiles it automatically.
3. Launches `lifelink_server.exe` on port `8080`.
4. Initializes `database/lifelink.db` with sample donors and requests if empty.
5. Automatically opens your default web browser to **`http://localhost:8080`**.

---

### Method 2: Manual Compilation & Execution

#### Step 1: Open Terminal in the `backend` folder
```cmd
cd backend
```

#### Step 2: Compile the Project
Using the automated build script:
```cmd
.\build.bat
```
*Or using the compiler directly:*
```cmd
g++ -std=c++17 -O2 -Iinclude db_manager.cpp server.cpp sqlite3.o -o lifelink_server.exe -lws2_32 -lmswsock
```

#### Step 3: Run the Server
```cmd
.\lifelink_server.exe
```

#### Step 4: Open in Web Browser
Navigate to:
```text
http://localhost:8080
```

---

## 🔑 Demo Accounts for Evaluation / Viva

You can log in immediately using these pre-configured accounts:

| Role | Email | Password | Purpose |
|---|---|---|---|
| **Admin** | `admin@lifelink.org` | `admin123` | Full administrative overview |
| **Donor (Rahul)** | `rahul.sharma@example.com` | `rahul123` | Active O+ donor profile |
| **Donor (Priya)** | `priya.patel@example.com` | `priya123` | Active A+ donor profile |

> **Tip:** On the `login.html` page, click the **"Demo Admin"** or **"Demo Donor"** buttons to automatically fill the credentials with one click.

---

## 🛠️ C++ Technical Architecture (For Beginners)

If you are presenting this project for your B.Tech internship, here is an explanation of every component:

### 1. Why C++ for Backend?
- **Speed & Latency:** In blood emergencies, every millisecond counts. C++ compiles directly to native x64 machine code without runtime overhead or garbage collection pauses.
- **Low Memory Footprint:** The entire compiled server executable is only ~4.8 MB and runs with minimal RAM usage.
- **Multi-threaded Concurrency:** Crow leverages Asio's asynchronous I/O and thread pools (`.multithreaded()`) to handle concurrent client requests smoothly.

### 2. How Crow Works (`backend/server.cpp`)
Crow uses a macro syntax similar to Python's Flask:
```cpp
CROW_ROUTE(app, "/api/donors").methods(crow::HTTPMethod::Get)([&db](const crow::request& req) {
    std::string bg = req.url_params.get("blood_group") ? req.url_params.get("blood_group") : "";
    std::string city = req.url_params.get("city") ? req.url_params.get("city") : "";
    auto donors = db.getDonors(bg, city);
    // returns JSON response with CORS headers
});
```

### 3. How SQLite Works (`backend/db_manager.cpp`)
Instead of raw string concatenation (which causes SQL injection), we use **SQLite Prepared Statements**:
```cpp
sqlite3_stmt* stmt = nullptr;
sqlite3_prepare_v2(db_, "SELECT * FROM donors WHERE blood_group = ? AND is_available = 1;", -1, &stmt, nullptr);
sqlite3_bind_text(stmt, 1, bloodGroup.c_str(), -1, SQLITE_TRANSIENT);
while (sqlite3_step(stmt) == SQLITE_ROW) {
    // Read columns safely
}
sqlite3_finalize(stmt);
```

### 4. Database Tables (`database/schema.sql`)
1. **`users`**: Stores user authentication credentials, phone, and role.
2. **`donors`**: Stores donor profiles (blood group, age, gender, contact, city, state, availability).
3. **`blood_requests`**: Stores emergency requests (patient name, units needed, hospital, urgency level, status).

---

## 📡 REST API Reference

| Endpoint | Method | Description | Parameters / Body |
|---|---|---|---|
| `/api/stats` | `GET` | Returns overall system stats & blood group counts | None |
| `/api/donors` | `GET` | Search and filter donors | `?blood_group=O+&city=Delhi&available_only=1` |
| `/api/donors/<id>` | `GET` | Get details of a single donor | None |
| `/api/donors` | `POST` | Register a new donor | JSON `{name, blood_group, age, phone, city, ...}` |
| `/api/donors/<id>/availability` | `PUT` | Toggle availability status | JSON `{"is_available": 1}` |
| `/api/donors/<id>` | `DELETE` | Delete donor record | None |
| `/api/requests` | `GET` | List submitted blood requests | `?status=Pending&blood_group=A+` |
| `/api/requests` | `POST` | Submit an emergency blood request | JSON `{patient_name, blood_group, units_needed, hospital_name, city, contact_phone, urgency, ...}` |
| `/api/requests/<id>/status` | `PUT` | Update request status | JSON `{"status": "Fulfilled"}` |
| `/api/auth/register` | `POST` | Register user account | JSON `{name, email, password, phone}` |
| `/api/auth/login` | `POST` | Authenticate user | JSON `{email, password}` |

---

## 🌍 How to Share Your Website (Public URL)

You have two simple options to share LifeLink with friends, evaluators, and college faculty:

### Option 1: Instant 1-Click Sharing (Direct from your PC)
Run the included `share_public.bat` script while your C++ server is running:
```cmd
.\share_public.bat
```
- It uses Windows built-in OpenSSH to create an instant, secure public HTTPS URL (e.g. `https://xxxx.a.pinggy.link`).
- You can immediately share this link via WhatsApp or email.
- Anyone on mobile or desktop can open and test your full C++ website live!

### Option 2: 24/7 Permanent Free Cloud Hosting (Always Online)
To keep the website running permanently on the cloud (even when your laptop is turned off):
1. Push your project code to **GitHub**.
2. Go to **[Render.com](https://render.com/)** and sign up for free.
3. Click **"New +"** → **"Web Service"** and select your GitHub repository.
4. Render will automatically detect the included **`Dockerfile`**, compile the C++ server on Ubuntu Linux, and deploy it.
5. You get a permanent free public URL: `https://lifelink-blood-finder.onrender.com`.

---

## ❓ Frequently Asked Questions & Troubleshooting

### Q1: Can I open `frontend/index.html` directly in my browser without compiling C++?
**Yes!** If you double click `frontend/index.html` or open it with VS Code Live Server, the interface will display in preview mode. However, to execute actual database queries, registrations, and live searches, run the C++ backend via `run.bat` so the `/api` endpoints are active.

### Q2: What compiler is required?
Any modern C++ compiler supporting **C++17**:
- **GCC / MinGW-w64** (GCC 10 or later recommended; GCC 14.1.0 included in `w64devkit`).
- **Clang** (v11+).
- **MSVC** (Visual Studio 2019+).

### Q3: How do I stop the server?
Press `Ctrl + C` in the Command Prompt or PowerShell window where `lifelink_server.exe` is running.

---

## 📜 Internship Project Attribution
- **Project Name:** LifeLink – Blood Donor Finder
- **Technology Stack:** C++17, Crow Framework, SQLite3, HTML5, CSS3, JavaScript
- **Context:** B.Tech Computer Science & Engineering Internship Project
- **Purpose:** Voluntary, non-commercial emergency healthcare support.
