/**
 * Dashboard Component
 */

import { useState, useEffect } from 'react';
import api from '../services/api';
import './Dashboard.css';

function Dashboard({ ws }) {
  const [stats, setStats] = useState(null);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState(null);

  useEffect(() => {
    loadStats();
    
    // Subscribe to real-time updates
    const unsubscribe = ws.subscribe('detection', (data) => {
      // Refresh stats on new detection
      loadStats();
    });

    return () => unsubscribe();
  }, [ws]);

  const loadStats = async () => {
    try {
      const data = await api.getDashboardStats();
      setStats(data);
      setError(null);
    } catch (err) {
      setError(err.message);
    } finally {
      setLoading(false);
    }
  };

  if (loading) {
    return <div className="loading">Loading dashboard...</div>;
  }

  if (error) {
    return <div className="error-message">Error: {error}</div>;
  }

  return (
    <div className="dashboard fade-in">
      <div className="page-header">
        <h1>Dashboard</h1>
        <span className="live-indicator">Live</span>
      </div>

      {/* Stats Grid */}
      <div className="stats-grid">
        <div className="stat-card">
          <div className="stat-value">{stats?.overview?.totalPlayers || 0}</div>
          <div className="stat-label">Total Players</div>
        </div>
        <div className="stat-card">
          <div className="stat-value text-error">{stats?.overview?.activeBans || 0}</div>
          <div className="stat-label">Active Bans</div>
        </div>
        <div className="stat-card">
          <div className="stat-value text-warning">{stats?.overview?.todayDetections || 0}</div>
          <div className="stat-label">Today's Detections</div>
        </div>
        <div className="stat-card">
          <div className="stat-value">{stats?.overview?.weekDetections || 0}</div>
          <div className="stat-label">This Week</div>
        </div>
      </div>

      {/* Main Content Grid */}
      <div className="dashboard-grid">
        {/* Recent Detections */}
        <div className="card">
          <div className="card-header">
            <h3 className="card-title">Recent Detections</h3>
          </div>
          <div className="detection-list">
            {stats?.recentDetections?.length === 0 ? (
              <div className="empty-state">No recent detections</div>
            ) : (
              stats?.recentDetections?.map(detection => (
                <div key={detection.id} className="detection-item">
                  <div className="detection-info">
                    <span className="detection-player">{detection.player_name}</span>
                    <span className="detection-type">{detection.detection_type}</span>
                  </div>
                  <div className="detection-meta">
                    <span className={`badge badge-${getSeverityClass(detection.severity)}`}>
                      {detection.severity}
                    </span>
                    <span className="detection-time">
                      {formatTime(detection.timestamp)}
                    </span>
                  </div>
                </div>
              ))
            )}
          </div>
        </div>

        {/* Recent Sanctions */}
        <div className="card">
          <div className="card-header">
            <h3 className="card-title">Recent Sanctions</h3>
          </div>
          <div className="sanction-list">
            {stats?.recentSanctions?.length === 0 ? (
              <div className="empty-state">No recent sanctions</div>
            ) : (
              stats?.recentSanctions?.map(sanction => (
                <div key={sanction.id} className="sanction-item">
                  <div className="sanction-info">
                    <span className="sanction-player">{sanction.player_name}</span>
                    <span className="sanction-reason">{sanction.reason}</span>
                  </div>
                  <div className="sanction-meta">
                    <span className={`badge badge-${getSanctionClass(sanction.sanction_type)}`}>
                      {sanction.sanction_type}
                    </span>
                    <span className="sanction-time">
                      {formatTime(sanction.created_at)}
                    </span>
                  </div>
                </div>
              ))
            )}
          </div>
        </div>
      </div>

      {/* Active Ruleset */}
      {stats?.activeRuleset && (
        <div className="card ruleset-card">
          <div className="card-header">
            <h3 className="card-title">Active Ruleset</h3>
            <span className="badge badge-success">Active</span>
          </div>
          <div className="ruleset-info">
            <strong>{stats.activeRuleset.name}</strong>
            <span className="text-muted"> v{stats.activeRuleset.version}</span>
            <p className="text-muted mt-sm">
              Last updated: {formatTime(stats.activeRuleset.updated_at)}
            </p>
          </div>
        </div>
      )}
    </div>
  );
}

function getSeverityClass(severity) {
  switch (severity) {
    case 'low': return 'success';
    case 'medium': return 'warning';
    case 'high': return 'error';
    case 'critical': return 'error';
    default: return 'info';
  }
}

function getSanctionClass(type) {
  switch (type) {
    case 'warn': return 'warning';
    case 'kick': return 'warning';
    case 'ban': return 'error';
    default: return 'info';
  }
}

function formatTime(timestamp) {
  const date = new Date(timestamp);
  const now = new Date();
  const diff = now - date;
  
  if (diff < 60000) return 'Just now';
  if (diff < 3600000) return `${Math.floor(diff / 60000)}m ago`;
  if (diff < 86400000) return `${Math.floor(diff / 3600000)}h ago`;
  return date.toLocaleDateString();
}

export default Dashboard;
