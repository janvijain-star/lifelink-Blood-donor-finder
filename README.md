# LifeLink – Every Drop Can Save a Life

A multi-page, responsive **website** (runs in any web browser) that connects blood donors with patients.

- **Frontend:** HTML, CSS, JavaScript (`frontend/`)
- **Backend:** C++17 with the Crow web framework (`backend/`)
- **Database:** SQLite (`database/`)

No Node.js, Python or PHP is used. The C++ server serves both the website pages and the REST API.

## Folder structure
```
LifeLink/
├── backend/   main.cpp, CMakeLists.txt, include/crow_all.h
├── database/  schema.sql, seed.sql (lifelink.db is created on first run)
└── frontend/  index.html, register.html, find.html, request.html, dashboard.html, css/, js/
```

## Pages
| Page | What it does |
|---|---|
| Home (`index.html`) | Hero, Find/Become a Donor buttons, info, live statistics |
| Become a Donor (`register.html`) | Registration form with validation; edit/delete your own profile via your mobile number |
| Find a Donor (`find.html`) | Filter by blood group and city; contacts masked unless donor consented |
| Emergency Request (`request.html`) | Submit a blood request |
| Dashboard (`dashboard.html`) | Totals, pending/fulfilled, donors per group, all requests, "My requests" with mark-fulfilled/delete |

## Install dependencies

### Ubuntu / Debian / WSL
```bash
sudo apt update
sudo apt install -y build-essential cmake libsqlite3-dev libasio-dev
```
### macOS
```bash
brew install cmake sqlite asio
```
### Windows
Use **WSL (Ubuntu)** with the commands above (easiest), or MSYS2:
`pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-sqlite3 mingw-w64-x86_64-asio`

Crow is header-only and already included at `backend/include/crow_all.h` (v1.2.0).

## Build and run
```bash
cd backend
mkdir build && cd build
cmake ..
make
./lifelink ../..
```
Open **http://localhost:18080** in your browser.

The argument `../..` is the project root (folder that contains `frontend/` and `database/`).
On first run the database `database/lifelink.db` is created and filled with 12 sample donors and 4 sample requests.
Delete that file to reset the data.

Without CMake:
```bash
g++ -std=c++17 backend/main.cpp -Ibackend/include -lsqlite3 -lpthread -o lifelink
./lifelink .
```

## REST API
| Method | Endpoint | Purpose |
|---|---|---|
| POST | `/api/donors` | Register donor (Create) |
| GET | `/api/donors?blood_group=O%2B&city=Delhi&available=1` | Search donors (Read, contacts masked) |
| GET | `/api/donors/me?mobile=...` | Load own profile |
| PUT | `/api/donors/{id}` | Update donor (registered mobile must match) |
| DELETE | `/api/donors/{id}?mobile=...` | Delete donor (registered mobile must match) |
| POST | `/api/donors/{id}/contact` | Reveal contact — only if donor consented; requester name/phone logged |
| POST | `/api/requests` | Create emergency request |
| GET | `/api/requests?status=Pending` / `?contact=...` | List all / my requests |
| PUT | `/api/requests/{id}` | Mark Pending/Fulfilled (contact must match) |
| DELETE | `/api/requests/{id}?contact=...` | Delete request (contact must match) |
| GET | `/api/stats` | Dashboard numbers |

## Privacy protection
- Donors choose whether their contact details may be shared (consent checkbox).
- Search results always show masked phone/email (e.g. `98XXXXXX10`).
- Full contact is revealed only for consenting donors, after the requester enters their name and phone; each view is stored in `contact_log`.
- Request contact numbers are masked in public lists.
- All SQL uses prepared statements (prevents SQL injection); inputs validated on both browser and server.

## Sample test data
- Search **O+** in **Delhi** → Aarav Sharma (consented: click "View contact").
- Search **B+** in **Mumbai** → Rohan Mehta (no consent: contact stays hidden).
- Dashboard → "My Submitted Requests" with **9111222333** shows 2 sample requests.
- Edit a donor: on Become a Donor click "Already registered?" and enter **9876543210**.
