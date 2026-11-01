-- Sample data for testing (fictional people)
INSERT INTO donors (full_name, age, gender, mobile, email, blood_group, city, state, available, consent_share_contact) VALUES
('Aarav Sharma',   24, 'Male',   '9876543210', 'aarav.sharma@example.com',  'O+',  'Delhi',     'Delhi',          1, 1),
('Priya Verma',    29, 'Female', '9812345678', 'priya.verma@example.com',   'A+',  'Delhi',     'Delhi',          1, 1),
('Rohan Mehta',    35, 'Male',   '9988776655', 'rohan.mehta@example.com',   'B+',  'Mumbai',    'Maharashtra',    1, 0),
('Sneha Iyer',     27, 'Female', '9765432109', 'sneha.iyer@example.com',    'AB+', 'Chennai',   'Tamil Nadu',     1, 1),
('Karan Singh',    31, 'Male',   '9123456780', 'karan.singh@example.com',   'O-',  'Jaipur',    'Rajasthan',      1, 1),
('Ananya Gupta',   22, 'Female', '9001122334', 'ananya.gupta@example.com',  'A-',  'Lucknow',   'Uttar Pradesh',  0, 1),
('Vikram Rao',     40, 'Male',   '9345678901', 'vikram.rao@example.com',    'B-',  'Bengaluru', 'Karnataka',      1, 1),
('Meera Nair',     33, 'Female', '9456789012', 'meera.nair@example.com',    'O+',  'Kochi',     'Kerala',         1, 0),
('Arjun Patel',    26, 'Male',   '9567890123', 'arjun.patel@example.com',   'AB-', 'Ahmedabad', 'Gujarat',        1, 1),
('Isha Kapoor',    30, 'Female', '9678901234', 'isha.kapoor@example.com',   'B+',  'Mumbai',    'Maharashtra',    1, 1),
('Rahul Jain',     28, 'Male',   '9789012345', 'rahul.jain@example.com',    'A+',  'Indore',    'Madhya Pradesh', 1, 1),
('Divya Reddy',    25, 'Female', '9890123456', 'divya.reddy@example.com',   'O+',  'Hyderabad', 'Telangana',      1, 1);

INSERT INTO blood_requests (patient_name, blood_group, hospital, city, contact, units, emergency_level, required_date, status) VALUES
('Suresh Kumar', 'O+',  'AIIMS',                 'Delhi',     '9111222333', 2, 'Critical', '2026-11-03', 'Pending'),
('Lakshmi Devi', 'B+',  'KEM Hospital',          'Mumbai',    '9222333444', 1, 'High',     '2026-11-05', 'Pending'),
('Mohan Das',    'A-',  'SGPGI',                 'Lucknow',   '9333444555', 3, 'Normal',   '2026-11-10', 'Fulfilled'),
('Fatima Shaikh','AB+', 'Apollo Hospital',       'Chennai',   '9111222333', 1, 'High',     '2026-11-04', 'Fulfilled');
