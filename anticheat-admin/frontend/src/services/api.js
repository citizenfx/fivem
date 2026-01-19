/**
 * API Service
 * Handles all HTTP requests to the backend
 */

const API_BASE = '/api';

class ApiService {
  constructor() {
    this.token = localStorage.getItem('token');
  }

  setToken(token) {
    this.token = token;
    if (token) {
      localStorage.setItem('token', token);
    } else {
      localStorage.removeItem('token');
    }
  }

  async request(endpoint, options = {}) {
    const url = `${API_BASE}${endpoint}`;
    const headers = {
      'Content-Type': 'application/json',
      ...options.headers,
    };

    if (this.token) {
      headers['Authorization'] = `Bearer ${this.token}`;
    }

    const response = await fetch(url, {
      ...options,
      headers,
    });

    if (response.status === 401) {
      this.setToken(null);
      window.location.href = '/login';
      throw new Error('Unauthorized');
    }

    const data = await response.json();

    if (!response.ok) {
      throw new Error(data.error || 'Request failed');
    }

    return data;
  }

  // Auth
  async login(username, password) {
    const data = await this.request('/auth/login', {
      method: 'POST',
      body: JSON.stringify({ username, password }),
    });
    this.setToken(data.token);
    return data;
  }

  logout() {
    this.setToken(null);
    window.location.href = '/login';
  }

  async getMe() {
    return this.request('/auth/me');
  }

  // Stats
  async getDashboardStats() {
    return this.request('/stats/dashboard');
  }

  // Players
  async getPlayers(params = {}) {
    const query = new URLSearchParams(params).toString();
    return this.request(`/players?${query}`);
  }

  async getPlayer(license) {
    return this.request(`/players/${encodeURIComponent(license)}`);
  }

  async updatePlayer(license, data) {
    return this.request(`/players/${encodeURIComponent(license)}`, {
      method: 'PATCH',
      body: JSON.stringify(data),
    });
  }

  // Detections
  async getDetections(params = {}) {
    const query = new URLSearchParams(params).toString();
    return this.request(`/detections?${query}`);
  }

  async getDetectionStats(days = 7) {
    return this.request(`/detections/stats?days=${days}`);
  }

  // Sanctions
  async getSanctions(params = {}) {
    const query = new URLSearchParams(params).toString();
    return this.request(`/sanctions?${query}`);
  }

  async createSanction(data) {
    return this.request('/sanctions', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  async updateSanction(id, data) {
    return this.request(`/sanctions/${id}`, {
      method: 'PATCH',
      body: JSON.stringify(data),
    });
  }

  // Rulesets
  async getRulesets() {
    return this.request('/rulesets');
  }

  async getRuleset(id) {
    return this.request(`/rulesets/${id}`);
  }

  async createRuleset(data) {
    return this.request('/rulesets', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  async updateRuleset(id, data) {
    return this.request(`/rulesets/${id}`, {
      method: 'PUT',
      body: JSON.stringify(data),
    });
  }

  async activateRuleset(id) {
    return this.request(`/rulesets/${id}/activate`, {
      method: 'POST',
    });
  }

  async deleteRuleset(id) {
    return this.request(`/rulesets/${id}`, {
      method: 'DELETE',
    });
  }

  // Generic methods for admin routes
  async get(endpoint) {
    return this.request(endpoint);
  }

  async post(endpoint, data) {
    return this.request(endpoint, {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  async patch(endpoint, data) {
    return this.request(endpoint, {
      method: 'PATCH',
      body: JSON.stringify(data),
    });
  }

  async put(endpoint, data) {
    return this.request(endpoint, {
      method: 'PUT',
      body: JSON.stringify(data),
    });
  }

  async delete(endpoint) {
    return this.request(endpoint, {
      method: 'DELETE',
    });
  }
}

export const api = new ApiService();
export default api;
