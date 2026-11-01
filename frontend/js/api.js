/**
 * LifeLink - Blood Donor Finder
 * C++ Crow Backend API Integration Module
 */

const API_BASE = window.location.origin.includes('http') && !window.location.origin.startsWith('file:')
    ? window.location.origin
    : 'http://localhost:8080';

const LifeLinkAPI = {
    // --------------------------------------------------------------------------
    // Generic Request Helper
    // --------------------------------------------------------------------------
    async request(endpoint, options = {}) {
        const url = `${API_BASE}${endpoint}`;
        const defaultHeaders = {
            'Content-Type': 'application/json',
            'Accept': 'application/json'
        };

        const config = {
            ...options,
            headers: {
                ...defaultHeaders,
                ...(options.headers || {})
            }
        };

        try {
            const response = await fetch(url, config);
            const data = await response.json().catch(() => null);

            if (!response.ok) {
                const errorMsg = (data && data.message) ? data.message : `Server Error: HTTP ${response.status}`;
                throw new Error(errorMsg);
            }
            return data;
        } catch (err) {
            console.error(`[API ERROR] ${endpoint}:`, err);
            throw err;
        }
    },

    // --------------------------------------------------------------------------
    // System Statistics
    // --------------------------------------------------------------------------
    async getStats() {
        return await this.request('/api/stats');
    },

    // --------------------------------------------------------------------------
    // Donors API
    // --------------------------------------------------------------------------
    async getDonors(bloodGroup = '', city = '', availableOnly = 0) {
        const params = new URLSearchParams();
        if (bloodGroup && bloodGroup !== 'ALL') params.append('blood_group', bloodGroup);
        if (city && city.trim() !== '') params.append('city', city.trim());
        if (availableOnly) params.append('available_only', '1');

        const queryString = params.toString() ? `?${params.toString()}` : '';
        return await this.request(`/api/donors${queryString}`);
    },

    async getDonorById(id) {
        return await this.request(`/api/donors/${id}`);
    },

    async registerDonor(donorData) {
        return await this.request('/api/donors', {
            method: 'POST',
            body: JSON.stringify(donorData)
        });
    },

    async updateDonorAvailability(id, isAvailable) {
        return await this.request(`/api/donors/${id}/availability`, {
            method: 'PUT',
            body: JSON.stringify({ is_available: isAvailable ? 1 : 0 })
        });
    },

    async deleteDonor(id) {
        return await this.request(`/api/donors/${id}`, {
            method: 'DELETE'
        });
    },

    // --------------------------------------------------------------------------
    // Blood Requests API
    // --------------------------------------------------------------------------
    async getBloodRequests(status = '', bloodGroup = '', city = '') {
        const params = new URLSearchParams();
        if (status && status !== 'ALL') params.append('status', status);
        if (bloodGroup && bloodGroup !== 'ALL') params.append('blood_group', bloodGroup);
        if (city && city.trim() !== '') params.append('city', city.trim());

        const queryString = params.toString() ? `?${params.toString()}` : '';
        return await this.request(`/api/requests${queryString}`);
    },

    async getBloodRequestById(id) {
        return await this.request(`/api/requests/${id}`);
    },

    async submitBloodRequest(requestData) {
        return await this.request('/api/requests', {
            method: 'POST',
            body: JSON.stringify(requestData)
        });
    },

    async updateBloodRequestStatus(id, newStatus) {
        return await this.request(`/api/requests/${id}/status`, {
            method: 'PUT',
            body: JSON.stringify({ status: newStatus })
        });
    },

    async deleteBloodRequest(id) {
        return await this.request(`/api/requests/${id}`, {
            method: 'DELETE'
        });
    },

    // --------------------------------------------------------------------------
    // Authentication & Users API
    // --------------------------------------------------------------------------
    async registerUser(userData) {
        return await this.request('/api/auth/register', {
            method: 'POST',
            body: JSON.stringify(userData)
        });
    },

    async loginUser(email, password) {
        return await this.request('/api/auth/login', {
            method: 'POST',
            body: JSON.stringify({ email, password })
        });
    }
};

window.LifeLinkAPI = LifeLinkAPI;
