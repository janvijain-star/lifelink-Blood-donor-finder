-- LifeLink database schema (SQLite)
CREATE TABLE IF NOT EXISTS donors (
    id                     INTEGER PRIMARY KEY AUTOINCREMENT,
    full_name              TEXT    NOT NULL,
    age                    INTEGER NOT NULL CHECK (age BETWEEN 18 AND 65),
    gender                 TEXT    NOT NULL CHECK (gender IN ('Male','Female','Other')),
    mobile                 TEXT    NOT NULL UNIQUE,
    email                  TEXT    NOT NULL,
    blood_group            TEXT    NOT NULL CHECK (blood_group IN ('A+','A-','B+','B-','AB+','AB-','O+','O-')),
    city                   TEXT    NOT NULL,
    state                  TEXT    NOT NULL,
    available              INTEGER NOT NULL DEFAULT 1,
    consent_share_contact  INTEGER NOT NULL DEFAULT 0,
    created_at             TEXT    NOT NULL DEFAULT (datetime('now'))
);

CREATE TABLE IF NOT EXISTS blood_requests (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    patient_name     TEXT    NOT NULL,
    blood_group      TEXT    NOT NULL CHECK (blood_group IN ('A+','A-','B+','B-','AB+','AB-','O+','O-')),
    hospital         TEXT    NOT NULL,
    city             TEXT    NOT NULL,
    contact          TEXT    NOT NULL,
    units            INTEGER NOT NULL CHECK (units BETWEEN 1 AND 10),
    emergency_level  TEXT    NOT NULL CHECK (emergency_level IN ('Critical','High','Normal')),
    required_date    TEXT    NOT NULL,
    status           TEXT    NOT NULL DEFAULT 'Pending' CHECK (status IN ('Pending','Fulfilled')),
    created_at       TEXT    NOT NULL DEFAULT (datetime('now'))
);

-- Audit trail: who viewed a donor's contact details
CREATE TABLE IF NOT EXISTS contact_log (
    id               INTEGER PRIMARY KEY AUTOINCREMENT,
    donor_id         INTEGER NOT NULL REFERENCES donors(id) ON DELETE CASCADE,
    requester_name   TEXT    NOT NULL,
    requester_phone  TEXT    NOT NULL,
    viewed_at        TEXT    NOT NULL DEFAULT (datetime('now'))
);

CREATE INDEX IF NOT EXISTS idx_donor_search ON donors(blood_group, city);
CREATE INDEX IF NOT EXISTS idx_request_contact ON blood_requests(contact);
