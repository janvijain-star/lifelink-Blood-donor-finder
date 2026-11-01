/**
 * LifeLink - Blood Donor Finder
 * Main Application Frontend Logic
 */

document.addEventListener('DOMContentLoaded', () => {
    initNavigation();
    initAuthUI();
    detectPageAndInit();
});

// --------------------------------------------------------------------------
// Navigation & Mobile Menu
// --------------------------------------------------------------------------
function initNavigation() {
    const mobileToggle = document.getElementById('mobileToggle');
    const navMenu = document.getElementById('navMenu');

    if (mobileToggle && navMenu) {
        mobileToggle.addEventListener('click', () => {
            navMenu.classList.toggle('active');
        });
    }

    // Close menu when clicking outside
    document.addEventListener('click', (e) => {
        if (navMenu && navMenu.classList.contains('active')) {
            if (!navMenu.contains(e.target) && !mobileToggle.contains(e.target)) {
                navMenu.classList.remove('active');
            }
        }
    });
}

// --------------------------------------------------------------------------
// Authentication Session Management
// --------------------------------------------------------------------------
function getCurrentUser() {
    try {
        const stored = localStorage.getItem('lifelink_user');
        return stored ? JSON.parse(stored) : null;
    } catch {
        return null;
    }
}

function setCurrentUser(user) {
    if (user) {
        localStorage.setItem('lifelink_user', JSON.stringify(user));
    } else {
        localStorage.removeItem('lifelink_user');
    }
    initAuthUI();
}

function logoutUser() {
    setCurrentUser(null);
    showToast('You have been logged out successfully.', 'info');
    setTimeout(() => {
        window.location.href = 'index.html';
    }, 800);
}

function initAuthUI() {
    const navActions = document.getElementById('navAuthActions');
    if (!navActions) return;

    const user = getCurrentUser();
    if (user) {
        navActions.innerHTML = `
            <div style="display: flex; align-items: center; gap: 0.75rem;">
                <span class="user-greeting" style="font-size: 0.9rem; font-weight: 600; color: var(--gray-700);">
                    👤 ${escapeHtml(user.name.split(' ')[0])}
                </span>
                <button onclick="logoutUser()" class="btn btn-secondary btn-sm" title="Log Out">
                    Logout
                </button>
            </div>
        `;
    } else {
        navActions.innerHTML = `
            <a href="login.html" class="btn btn-secondary btn-sm">Login</a>
            <a href="register.html" class="btn btn-primary btn-sm">Join as Donor</a>
        `;
    }
}

// --------------------------------------------------------------------------
// Toast Notifications
// --------------------------------------------------------------------------
function showToast(message, type = 'info', duration = 4000) {
    let container = document.getElementById('toast-container');
    if (!container) {
        container = document.createElement('div');
        container.id = 'toast-container';
        document.body.appendChild(container);
    }

    const toast = document.createElement('div');
    toast.className = `toast ${type}`;

    const icons = {
        success: '✅',
        error: '⚠️',
        warning: '⚡',
        info: 'ℹ️'
    };

    toast.innerHTML = `
        <span class="toast-icon">${icons[type] || 'ℹ️'}</span>
        <span class="toast-message">${escapeHtml(message)}</span>
        <button class="toast-close" onclick="this.parentElement.remove()">&times;</button>
    `;

    container.appendChild(toast);

    setTimeout(() => {
        if (toast.parentElement) {
            toast.style.opacity = '0';
            toast.style.transform = 'translateX(100%)';
            setTimeout(() => toast.remove(), 300);
        }
    }, duration);
}

// --------------------------------------------------------------------------
// Page Router
// --------------------------------------------------------------------------
function detectPageAndInit() {
    const path = window.location.pathname.toLowerCase();

    if (path.endsWith('donors.html')) {
        initDonorsPage();
    } else if (path.endsWith('request.html')) {
        initRequestPage();
    } else if (path.endsWith('dashboard.html')) {
        initDashboardPage();
    } else if (path.endsWith('login.html')) {
        initLoginPage();
    } else if (path.endsWith('register.html')) {
        initRegisterPage();
    } else {
        initHomePage();
    }
}

// --------------------------------------------------------------------------
// 1. Home Page Initialization
// --------------------------------------------------------------------------
async function initHomePage() {
    // Quick search form handler
    const heroSearchForm = document.getElementById('heroSearchForm');
    if (heroSearchForm) {
        heroSearchForm.addEventListener('submit', (e) => {
            e.preventDefault();
            const bg = document.getElementById('heroBloodGroup')?.value || '';
            const city = document.getElementById('heroCity')?.value || '';
            window.location.href = `donors.html?bg=${encodeURIComponent(bg)}&city=${encodeURIComponent(city)}`;
        });
    }

    // Fetch stats and update hero ticker
    try {
        const stats = await LifeLinkAPI.getStats();
        if (stats && stats.success) {
            updateCounter('statTotalDonors', stats.total_donors || 10);
            updateCounter('statFulfilled', stats.fulfilled_requests || 12);
            updateCounter('statActiveRequests', stats.pending_requests || 4);
            updateCounter('statUnitsNeeded', stats.total_units_needed || 8);
        }
    } catch (err) {
        console.warn('Backend server not connected or offline. Displaying demo metrics.');
    }

    // Load urgent emergency requests on home page
    loadHomeUrgentRequests();
}

async function loadHomeUrgentRequests() {
    const listEl = document.getElementById('homeUrgentList');
    if (!listEl) return;

    try {
        const data = await LifeLinkAPI.getBloodRequests('Pending');
        if (data && data.requests && data.requests.length > 0) {
            const urgentOnes = data.requests.slice(0, 3);
            listEl.innerHTML = urgentOnes.map(r => `
                <div class="request-card ${r.urgency.toLowerCase()}">
                    <div class="request-top">
                        <span class="urgency-badge urgency-${r.urgency.toLowerCase()}">
                            ${r.urgency === 'Critical' ? '🚨 CRITICAL' : '⚡ ' + r.urgency}
                        </span>
                        <span class="request-units-pill">${r.units_needed} Units ${r.blood_group}</span>
                    </div>
                    <h3 class="request-patient">${escapeHtml(r.patient_name)}</h3>
                    <div class="request-hospital">
                        🏥 ${escapeHtml(r.hospital_name)}, ${escapeHtml(r.city)}
                    </div>
                    <div class="request-notes">
                        "${escapeHtml(r.notes || 'Emergency donor required urgently.')}"
                    </div>
                    <div class="request-footer">
                        <span style="font-size: 0.85rem; color: var(--gray-500);">Contact: ${escapeHtml(r.contact_person)}</span>
                        <a href="tel:${encodeURIComponent(r.contact_phone)}" class="btn btn-danger btn-sm">
                            📞 Call Now
                        </a>
                    </div>
                </div>
            `).join('');
        } else {
            listEl.innerHTML = `
                <div style="grid-column: 1/-1; text-align: center; padding: 2rem; color: var(--gray-500);">
                    No urgent pending blood requests right now. All requests are currently fulfilled!
                </div>
            `;
        }
    } catch {
        listEl.innerHTML = `
            <div style="grid-column: 1/-1; text-align: center; padding: 2rem; color: var(--gray-500);">
                Connect C++ backend (run lifelink_server.exe) to view live requests.
            </div>
        `;
    }
}

// --------------------------------------------------------------------------
// 2. Donors Page Initialization
// --------------------------------------------------------------------------
let allDonors = [];
let currentView = 'cards';

async function initDonorsPage() {
    // Parse query params from URL
    const urlParams = new URLSearchParams(window.location.search);
    const initialBg = urlParams.get('bg') || '';
    const initialCity = urlParams.get('city') || '';

    const bgSelect = document.getElementById('filterBloodGroup');
    const cityInput = document.getElementById('filterCity');
    const availOnlyCheckbox = document.getElementById('filterAvailableOnly');

    if (bgSelect && initialBg) bgSelect.value = initialBg;
    if (cityInput && initialCity) cityInput.value = initialCity;

    // View toggles
    const btnCards = document.getElementById('viewCardsBtn');
    const btnTable = document.getElementById('viewTableBtn');

    if (btnCards && btnTable) {
        btnCards.addEventListener('click', () => {
            currentView = 'cards';
            btnCards.classList.add('active');
            btnTable.classList.remove('active');
            renderDonors();
        });
        btnTable.addEventListener('click', () => {
            currentView = 'table';
            btnTable.classList.add('active');
            btnCards.classList.remove('active');
            renderDonors();
        });
    }

    // Attach event listeners for real-time filtering
    if (bgSelect) bgSelect.addEventListener('change', fetchAndRenderDonors);
    if (cityInput) cityInput.addEventListener('input', debounce(fetchAndRenderDonors, 300));
    if (availOnlyCheckbox) availOnlyCheckbox.addEventListener('change', fetchAndRenderDonors);

    // Initial fetch
    await fetchAndRenderDonors();
}

async function fetchAndRenderDonors() {
    const bg = document.getElementById('filterBloodGroup')?.value || '';
    const city = document.getElementById('filterCity')?.value || '';
    const avail = document.getElementById('filterAvailableOnly')?.checked ? 1 : 0;

    const container = document.getElementById('donorsContainer');
    if (container) {
        container.innerHTML = `
            <div style="text-align: center; padding: 3rem; grid-column: 1/-1;">
                <div style="font-size: 2rem; margin-bottom: 0.5rem;">🩸</div>
                <p style="color: var(--gray-600);">Searching LifeLink donor network...</p>
            </div>
        `;
    }

    try {
        // Update Compatibility Helper Notice
        const noticeEl = document.getElementById('compatibilityNotice');
        if (noticeEl) {
            const compatInfo = {
                'O-': '🩸 <strong>O- is the Universal Donor</strong>: Can donate to ALL blood groups (O-, O+, A-, A+, B-, B+, AB-, AB+). Can only receive from O-.',
                'O+': '🩸 <strong>O+ Compatibility</strong>: Can donate to O+, A+, B+, AB+. Can receive from O+ and O-.',
                'A-': '🩸 <strong>A- Compatibility</strong>: Can donate to A-, A+, AB-, AB+. Can receive from A- and O-.',
                'A+': '🩸 <strong>A+ Compatibility</strong>: Can donate to A+ and AB+. Can receive from A+, A-, O+, O-.',
                'B-': '🩸 <strong>B- Compatibility</strong>: Can donate to B-, B+, AB-, AB+. Can receive from B- and O-.',
                'B+': '🩸 <strong>B+ Compatibility</strong>: Can donate to B+ and AB+. Can receive from B+, B-, O+, O-.',
                'AB-': '🩸 <strong>AB- Compatibility</strong>: Can donate to AB- and AB+. Can receive from AB-, A-, B-, O-.',
                'AB+': '🩸 <strong>AB+ is the Universal Recipient</strong>: Can receive from ALL blood groups. Can donate to AB+.'
            };
            if (bg && bg !== 'ALL' && compatInfo[bg]) {
                noticeEl.innerHTML = compatInfo[bg];
                noticeEl.style.display = 'block';
            } else {
                noticeEl.style.display = 'none';
            }
        }

        const response = await LifeLinkAPI.getDonors(bg, city, avail);
        allDonors = (response && response.donors) ? response.donors : [];
        updateDonorCountDisplay(allDonors.length);
        renderDonors();
    } catch (err) {
        if (container) {
            container.innerHTML = `
                <div style="text-align: center; padding: 3rem; grid-column: 1/-1; background: var(--white); border-radius: var(--radius-lg); border: 1px dashed var(--gray-300);">
                    <div style="font-size: 2rem; margin-bottom: 0.5rem; color: var(--danger);">⚠️</div>
                    <h3 style="margin-bottom: 0.5rem;">Unable to connect to C++ Backend</h3>
                    <p style="color: var(--gray-600); margin-bottom: 1rem;">
                        Please make sure the LifeLink C++ Server is running on <code>http://localhost:8080</code>.
                    </p>
                    <button onclick="fetchAndRenderDonors()" class="btn btn-primary btn-sm">Retry Connection</button>
                </div>
            `;
        }
    }
}

function updateDonorCountDisplay(count) {
    const countEl = document.getElementById('donorCountBadge');
    if (countEl) {
        countEl.textContent = `${count} Donor${count === 1 ? '' : 's'} Found`;
    }
}

function renderDonors() {
    const container = document.getElementById('donorsContainer');
    if (!container) return;

    if (allDonors.length === 0) {
        container.innerHTML = `
            <div style="text-align: center; padding: 3.5rem 1.5rem; grid-column: 1/-1; background: var(--white); border-radius: var(--radius-lg); border: 1px dashed var(--gray-300);">
                <div style="font-size: 2.5rem; margin-bottom: 0.75rem;">🔍</div>
                <h3 style="font-size: 1.25rem; font-weight: 700; color: var(--gray-800); margin-bottom: 0.5rem;">No Donors Found</h3>
                <p style="color: var(--gray-600); max-width: 450px; margin: 0 auto 1.5rem;">
                    We couldn't find matching donors for this criteria. Try changing your blood group or city filter, or submit an emergency request.
                </p>
                <a href="request.html" class="btn btn-emergency btn-sm">🚨 Submit Emergency Blood Request</a>
            </div>
        `;
        return;
    }

    if (currentView === 'cards') {
        container.className = 'donors-grid';
        container.innerHTML = allDonors.map(donor => `
            <div class="donor-card">
                <div>
                    <div class="donor-header">
                        <div class="donor-blood-badge">${escapeHtml(donor.blood_group)}</div>
                        <div class="donor-info-meta">
                            <h3 class="donor-name">${escapeHtml(donor.name)}</h3>
                            <div class="donor-subtitle">
                                ${donor.age} yrs • ${escapeHtml(donor.gender)}
                            </div>
                        </div>
                        <span class="donor-status-pill ${donor.is_available ? 'status-available' : 'status-unavailable'}">
                            ${donor.is_available ? '● Available' : '○ Busy'}
                        </span>
                    </div>

                    <div class="donor-body">
                        <div class="donor-detail-row">
                            <svg fill="currentColor" viewBox="0 0 20 20"><path fill-rule="evenodd" d="M5.05 4.05a7 7 0 119.9 9.9L10 18.9l-4.95-4.95a7 7 0 010-9.9zM10 11a2 2 0 100-4 2 2 0 000 4z" clip-rule="evenodd"></path></svg>
                            <span>${escapeHtml(donor.city)}${donor.state ? ', ' + escapeHtml(donor.state) : ''}</span>
                        </div>
                        <div class="donor-detail-row">
                            <svg fill="currentColor" viewBox="0 0 20 20"><path d="M2 3a1 1 0 011-1h2.153a1 1 0 01.986.836l.74 4.435a1 1 0 01-.54 1.06l-1.548.773a11.037 11.037 0 006.105 6.105l.774-1.548a1 1 0 011.059-.54l4.435.74a1 1 0 01.836.986V17a1 1 0 01-1 1h-2C7.82 18 2 12.18 2 4V3z"></path></svg>
                            <span>${escapeHtml(donor.phone)}</span>
                        </div>
                        ${donor.last_donation_date ? `
                            <div class="donor-detail-row" style="font-size: 0.8rem; color: var(--gray-500);">
                                <span>🗓 Last donation: ${escapeHtml(donor.last_donation_date)}</span>
                            </div>
                        ` : ''}
                    </div>
                </div>

                <div class="donor-actions">
                    <a href="tel:${encodeURIComponent(donor.phone)}" class="btn btn-primary btn-sm btn-block">
                        📞 Call Donor
                    </a>
                    <button onclick="copyToClipboard('${escapeHtml(donor.phone)}')" class="btn btn-secondary btn-sm" title="Copy Phone Number">
                        📋 Copy
                    </button>
                </div>
            </div>
        `).join('');
    } else {
        // Table view
        container.className = 'table-responsive';
        container.innerHTML = `
            <table class="data-table">
                <thead>
                    <tr>
                        <th>Blood</th>
                        <th>Name</th>
                        <th>Age / Gender</th>
                        <th>Location</th>
                        <th>Contact</th>
                        <th>Status</th>
                        <th>Action</th>
                    </tr>
                </thead>
                <tbody>
                    ${allDonors.map(d => `
                        <tr>
                            <td><strong style="color: var(--primary); font-size: 1.1rem;">${escapeHtml(d.blood_group)}</strong></td>
                            <td><strong>${escapeHtml(d.name)}</strong></td>
                            <td>${d.age} / ${escapeHtml(d.gender)}</td>
                            <td>${escapeHtml(d.city)}</td>
                            <td>${escapeHtml(d.phone)}</td>
                            <td>
                                <span class="donor-status-pill ${d.is_available ? 'status-available' : 'status-unavailable'}">
                                    ${d.is_available ? 'Available' : 'Busy'}
                                </span>
                            </td>
                            <td>
                                <a href="tel:${encodeURIComponent(d.phone)}" class="btn btn-primary btn-sm">
                                    Call
                                </a>
                            </td>
                        </tr>
                    `).join('')}
                </tbody>
            </table>
        `;
    }
}

// --------------------------------------------------------------------------
// 3. Emergency Request Page Initialization
// --------------------------------------------------------------------------
async function initRequestPage() {
    const form = document.getElementById('emergencyRequestForm');
    if (form) {
        form.addEventListener('submit', async (e) => {
            e.preventDefault();

            const patientName = document.getElementById('reqPatientName')?.value.trim();
            const bloodGroup = document.getElementById('reqBloodGroup')?.value;
            const unitsNeeded = parseInt(document.getElementById('reqUnits')?.value || '1', 10);
            const hospitalName = document.getElementById('reqHospital')?.value.trim();
            const city = document.getElementById('reqCity')?.value.trim();
            const contactPerson = document.getElementById('reqContactName')?.value.trim();
            const contactPhone = document.getElementById('reqPhone')?.value.trim();
            const urgency = document.getElementById('reqUrgency')?.value || 'Urgent';
            const notes = document.getElementById('reqNotes')?.value.trim() || '';

            // Form validation
            if (!patientName || !bloodGroup || !hospitalName || !city || !contactPhone) {
                showToast('Please fill all mandatory fields.', 'error');
                return;
            }

            if (contactPhone.replace(/\D/g, '').length < 10) {
                showToast('Please enter a valid 10-digit emergency contact phone number.', 'error');
                return;
            }

            const payload = {
                patient_name: patientName,
                blood_group: bloodGroup,
                units_needed: unitsNeeded,
                hospital_name: hospitalName,
                city: city,
                contact_person: contactPerson || patientName,
                contact_phone: contactPhone,
                urgency: urgency,
                notes: notes
            };

            const submitBtn = form.querySelector('button[type="submit"]');
            if (submitBtn) {
                submitBtn.disabled = true;
                submitBtn.textContent = 'Submitting to Network...';
            }

            try {
                const res = await LifeLinkAPI.submitBloodRequest(payload);
                if (res && res.success) {
                    showToast('Emergency request broadcasted successfully!', 'success');
                    form.reset();
                    await loadSubmittedRequests();
                } else {
                    showToast(res.message || 'Failed to submit request.', 'error');
                }
            } catch (err) {
                showToast(err.message || 'Failed to submit blood request. Ensure C++ backend is running.', 'error');
            } finally {
                if (submitBtn) {
                    submitBtn.disabled = false;
                    submitBtn.textContent = '🚨 Broadcast Emergency Blood Request';
                }
            }
        });
    }

    // Filter controls for active requests
    const filterStatus = document.getElementById('reqFilterStatus');
    if (filterStatus) {
        filterStatus.addEventListener('change', loadSubmittedRequests);
    }

    // Initial load of submitted requests
    await loadSubmittedRequests();
}

async function loadSubmittedRequests() {
    const listEl = document.getElementById('submittedRequestsList');
    if (!listEl) return;

    const statusFilter = document.getElementById('reqFilterStatus')?.value || '';

    listEl.innerHTML = `
        <div style="text-align: center; padding: 2.5rem; grid-column: 1/-1;">
            <p style="color: var(--gray-600);">Loading blood requests from database...</p>
        </div>
    `;

    try {
        const response = await LifeLinkAPI.getBloodRequests(statusFilter);
        const reqs = response.requests || [];

        if (reqs.length === 0) {
            listEl.innerHTML = `
                <div style="text-align: center; padding: 3rem; grid-column: 1/-1; background: var(--white); border-radius: var(--radius-lg); border: 1px dashed var(--gray-300);">
                    <div style="font-size: 2rem; margin-bottom: 0.5rem;">📋</div>
                    <p style="color: var(--gray-600);">No requests match the selected status.</p>
                </div>
            `;
            return;
        }

        listEl.innerHTML = reqs.map(r => `
            <div class="request-card ${r.urgency.toLowerCase()}">
                <div class="request-top">
                    <div>
                        <span class="urgency-badge urgency-${r.urgency.toLowerCase()}">
                            ${r.urgency === 'Critical' ? '🚨 CRITICAL' : '⚡ ' + r.urgency}
                        </span>
                        <span class="donor-status-pill ${r.status === 'Fulfilled' ? 'status-available' : 'status-unavailable'}" style="margin-left: 0.4rem;">
                            ${escapeHtml(r.status)}
                        </span>
                    </div>
                    <span class="request-units-pill">${r.units_needed} Units ${r.blood_group}</span>
                </div>

                <h3 class="request-patient">${escapeHtml(r.patient_name)}</h3>
                <div class="request-hospital">
                    🏥 ${escapeHtml(r.hospital_name)}, ${escapeHtml(r.city)}
                </div>

                ${r.notes ? `
                    <div class="request-notes">
                        "${escapeHtml(r.notes)}"
                    </div>
                ` : ''}

                <div class="request-footer">
                    <div>
                        <div style="font-size: 0.85rem; font-weight: 600; color: var(--gray-800);">${escapeHtml(r.contact_person)}</div>
                        <div style="font-size: 0.8rem; color: var(--gray-500);">${escapeHtml(r.contact_phone)}</div>
                    </div>
                    <div style="display: flex; gap: 0.5rem;">
                        <a href="tel:${encodeURIComponent(r.contact_phone)}" class="btn btn-danger btn-sm">
                            📞 Call
                        </a>
                        ${r.status === 'Pending' ? `
                            <button onclick="markRequestFulfilled(${r.id})" class="btn btn-secondary btn-sm" title="Mark request fulfilled">
                                ✔ Fulfilled
                            </button>
                        ` : ''}
                    </div>
                </div>
            </div>
        `).join('');
    } catch {
        listEl.innerHTML = `
            <div style="text-align: center; padding: 2rem; grid-column: 1/-1; color: var(--gray-500);">
                Unable to load blood requests. Make sure C++ backend is running.
            </div>
        `;
    }
}

async function markRequestFulfilled(id) {
    if (!confirm('Mark this blood request as fulfilled?')) return;

    try {
        await LifeLinkAPI.updateBloodRequestStatus(id, 'Fulfilled');
        showToast('Request status updated to Fulfilled!', 'success');
        await loadSubmittedRequests();
    } catch (err) {
        showToast(err.message || 'Failed to update request status', 'error');
    }
}

// --------------------------------------------------------------------------
// 4. Dashboard Page Initialization
// --------------------------------------------------------------------------
async function initDashboardPage() {
    try {
        const stats = await LifeLinkAPI.getStats();
        if (stats && stats.success) {
            // Update KPI metrics
            const elTotalDonors = document.getElementById('dashTotalDonors');
            const elAvailDonors = document.getElementById('dashAvailDonors');
            const elPendingReqs = document.getElementById('dashPendingReqs');
            const elUnitsNeeded = document.getElementById('dashUnitsNeeded');

            if (elTotalDonors) elTotalDonors.textContent = stats.total_donors;
            if (elAvailDonors) elAvailDonors.textContent = stats.available_donors;
            if (elPendingReqs) elPendingReqs.textContent = stats.pending_requests;
            if (elUnitsNeeded) elUnitsNeeded.textContent = stats.total_units_needed;

            // Render 8 Blood Group Inventory boxes
            const bloodGrid = document.getElementById('dashBloodGrid');
            if (bloodGrid && stats.blood_groups) {
                bloodGrid.innerHTML = stats.blood_groups.map(bg => `
                    <a href="donors.html?bg=${encodeURIComponent(bg.blood_group)}" class="blood-box" title="View available ${bg.blood_group} donors">
                        <div class="blood-box-type">${bg.blood_group}</div>
                        <div class="blood-box-count">${bg.count}</div>
                        <div class="blood-box-desc">Available</div>
                    </a>
                `).join('');
            }

            // Render Recent Donors
            const donorsTbody = document.getElementById('dashRecentDonorsTbody');
            if (donorsTbody && stats.recent_donors) {
                if (stats.recent_donors.length === 0) {
                    donorsTbody.innerHTML = '<tr><td colspan="4" style="text-align: center; color: var(--gray-500);">No donors registered yet.</td></tr>';
                } else {
                    donorsTbody.innerHTML = stats.recent_donors.map(d => `
                        <tr>
                            <td><strong style="color: var(--primary);">${escapeHtml(d.blood_group)}</strong></td>
                            <td>${escapeHtml(d.name)}</td>
                            <td>${escapeHtml(d.city)}</td>
                            <td>
                                <a href="tel:${encodeURIComponent(d.phone)}" class="btn btn-secondary btn-sm" style="padding: 0.2rem 0.5rem; font-size: 0.8rem;">
                                    📞 Call
                                </a>
                            </td>
                        </tr>
                    `).join('');
                }
            }

            // Render Recent Emergency Requests
            const reqsList = document.getElementById('dashRecentRequestsList');
            if (reqsList && stats.recent_requests) {
                if (stats.recent_requests.length === 0) {
                    reqsList.innerHTML = '<div style="color: var(--gray-500); padding: 1rem; text-align: center;">No emergency requests.</div>';
                } else {
                    reqsList.innerHTML = stats.recent_requests.map(r => `
                        <div style="display: flex; justify-content: space-between; align-items: center; padding: 0.75rem 0; border-bottom: 1px solid var(--gray-100);">
                            <div>
                                <div style="font-weight: 700; color: var(--gray-900); font-size: 0.95rem;">
                                    ${escapeHtml(r.patient_name)}
                                    <span style="color: var(--primary); font-size: 0.85rem; margin-left: 0.35rem;">(${r.blood_group} • ${r.units_needed} units)</span>
                                </div>
                                <div style="font-size: 0.8rem; color: var(--gray-500);">
                                    🏥 ${escapeHtml(r.hospital_name)}, ${escapeHtml(r.city)}
                                </div>
                            </div>
                            <div>
                                <span class="urgency-badge urgency-${r.urgency.toLowerCase()}" style="font-size: 0.7rem;">
                                    ${r.urgency}
                                </span>
                            </div>
                        </div>
                    `).join('');
                }
            }
        }
    } catch (err) {
        showToast('Unable to fetch live dashboard statistics from C++ server.', 'warning');
    }
}

// --------------------------------------------------------------------------
// 5. Login Page Initialization
// --------------------------------------------------------------------------
function initLoginPage() {
    const form = document.getElementById('loginForm');
    if (!form) return;

    form.addEventListener('submit', async (e) => {
        e.preventDefault();
        const email = document.getElementById('loginEmail')?.value.trim();
        const password = document.getElementById('loginPassword')?.value;

        if (!email || !password) {
            showToast('Please enter both email and password.', 'error');
            return;
        }

        const btn = form.querySelector('button[type="submit"]');
        if (btn) {
            btn.disabled = true;
            btn.textContent = 'Verifying credentials...';
        }

        try {
            const res = await LifeLinkAPI.loginUser(email, password);
            if (res && res.success) {
                setCurrentUser(res.user);
                showToast(`Welcome back, ${res.user.name}!`, 'success');
                setTimeout(() => {
                    window.location.href = 'dashboard.html';
                }, 800);
            } else {
                showToast(res.message || 'Login failed.', 'error');
            }
        } catch (err) {
            showToast(err.message || 'Connection to C++ server failed.', 'error');
        } finally {
            if (btn) {
                btn.disabled = false;
                btn.textContent = 'Log In to LifeLink';
            }
        }
    });
}

function fillDemoLogin(email, password) {
    const emailField = document.getElementById('loginEmail');
    const passField = document.getElementById('loginPassword');
    if (emailField) emailField.value = email;
    if (passField) passField.value = password;
    showToast('Credentials filled! Click Log In.', 'info', 2000);
}

// --------------------------------------------------------------------------
// 6. Register Page Initialization
// --------------------------------------------------------------------------
function initRegisterPage() {
    const form = document.getElementById('registerForm');
    if (!form) return;

    form.addEventListener('submit', async (e) => {
        e.preventDefault();

        const name = document.getElementById('regName')?.value.trim();
        const email = document.getElementById('regEmail')?.value.trim();
        const password = document.getElementById('regPassword')?.value;
        const phone = document.getElementById('regPhone')?.value.trim();
        const age = parseInt(document.getElementById('regAge')?.value || '0', 10);
        const gender = document.getElementById('regGender')?.value || 'Male';
        const bloodGroup = document.getElementById('regBloodGroup')?.value;
        const city = document.getElementById('regCity')?.value.trim();
        const state = document.getElementById('regState')?.value.trim();
        const lastDonation = document.getElementById('regLastDonation')?.value || '';

        // Validation
        if (!name || !email || !password || !phone || !bloodGroup || !city) {
            showToast('Please fill all mandatory fields.', 'error');
            return;
        }

        if (age < 18 || age > 65) {
            showToast('Donor must be between 18 and 65 years old to donate blood.', 'error');
            return;
        }

        if (phone.replace(/\D/g, '').length < 10) {
            showToast('Please enter a valid 10-digit mobile number.', 'error');
            return;
        }

        const payload = {
            name,
            email,
            password,
            phone,
            age,
            gender,
            blood_group: bloodGroup,
            city,
            state,
            last_donation_date: lastDonation,
            is_available: 1
        };

        const btn = form.querySelector('button[type="submit"]');
        if (btn) {
            btn.disabled = true;
            btn.textContent = 'Registering Donor Profile...';
        }

        try {
            const res = await LifeLinkAPI.registerUser(payload);
            if (res && res.success) {
                setCurrentUser(res.user);
                showToast('Registration successful! Welcome to LifeLink!', 'success');
                setTimeout(() => {
                    window.location.href = 'dashboard.html';
                }, 1000);
            } else {
                showToast(res.message || 'Registration failed.', 'error');
            }
        } catch (err) {
            showToast(err.message || 'Registration failed. Ensure C++ backend is running.', 'error');
        } finally {
            if (btn) {
                btn.disabled = false;
                btn.textContent = 'Complete Registration & Save Lives';
            }
        }
    });
}

// --------------------------------------------------------------------------
// Utility Helpers
// --------------------------------------------------------------------------
function updateCounter(elementId, targetValue) {
    const el = document.getElementById(elementId);
    if (!el) return;

    let current = 0;
    const step = Math.max(1, Math.ceil(targetValue / 20));
    const timer = setInterval(() => {
        current += step;
        if (current >= targetValue) {
            el.textContent = targetValue + '+';
            clearInterval(timer);
        } else {
            el.textContent = current + '+';
        }
    }, 40);
}

function copyToClipboard(text) {
    navigator.clipboard.writeText(text).then(() => {
        showToast(`Copied phone number (${text}) to clipboard!`, 'success');
    }).catch(() => {
        showToast(`Phone: ${text}`, 'info');
    });
}

function escapeHtml(str) {
    if (!str) return '';
    return String(str)
        .replace(/&/g, '&amp;')
        .replace(/</g, '&lt;')
        .replace(/>/g, '&gt;')
        .replace(/"/g, '&quot;')
        .replace(/'/g, '&#039;');
}

function debounce(func, wait) {
    let timeout;
    return function (...args) {
        clearTimeout(timeout);
        timeout = setTimeout(() => func.apply(this, args), wait);
    };
}

// Global helpers for inline HTML event handlers
window.copyToClipboard = copyToClipboard;
window.fillDemoLogin = fillDemoLogin;
window.markRequestFulfilled = markRequestFulfilled;
window.logoutUser = logoutUser;
