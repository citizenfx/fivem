/**
 * Detections List Component
 */

import { useState, useEffect } from 'react';
import api from '../services/api';
import './Detections.css';

function Detections() {
  const [detections, setDetections] = useState([]);
  const [pagination, setPagination] = useState({ page: 1, totalPages: 1 });
  const [loading, setLoading] = useState(true);
  const [filters, setFilters] = useState({ type: '', severity: '' });

  useEffect(() => {
    loadDetections();
  }, [pagination.page, filters]);

  const loadDetections = async () => {
    setLoading(true);
    try {
      const data = await api.getDetections({ 
        page: pagination.page, 
        limit: 50,
        ...filters 
      });
      setDetections(data.detections);
      setPagination(data.pagination);
    } catch (err) {
      console.error('Failed to load detections:', err);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="detections-page fade-in">
      <div className="page-header">
        <h1>Detections</h1>
        <div className="filters">
          <select 
            className="input filter-select"
            value={filters.type}
            onChange={(e) => setFilters(f => ({ ...f, type: e.target.value }))}
          >
            <option value="">All Types</option>
            <option value="speed_speed">Speed</option>
            <option value="speed_teleport">Teleport</option>
            <option value="health_invalid_health">Invalid Health</option>
            <option value="weapon_blacklisted_weapon">Blacklisted Weapon</option>
            <option value="entity_unauthorized_spawn">Unauthorized Spawn</option>
            <option value="event_rate_limit">Rate Limit</option>
          </select>
          <select 
            className="input filter-select"
            value={filters.severity}
            onChange={(e) => setFilters(f => ({ ...f, severity: e.target.value }))}
          >
            <option value="">All Severities</option>
            <option value="low">Low</option>
            <option value="medium">Medium</option>
            <option value="high">High</option>
            <option value="critical">Critical</option>
          </select>
        </div>
      </div>

      <div className="table-container">
        <table className="table">
          <thead>
            <tr>
              <th>ID</th>
              <th>Player</th>
              <th>Type</th>
              <th>Severity</th>
              <th>Details</th>
              <th>Timestamp</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan="6" className="text-center">Loading...</td></tr>
            ) : detections.length === 0 ? (
              <tr><td colSpan="6" className="text-center">No detections found</td></tr>
            ) : (
              detections.map(detection => (
                <tr key={detection.id}>
                  <td>#{detection.id}</td>
                  <td><strong>{detection.player_name}</strong></td>
                  <td><code>{detection.detection_type}</code></td>
                  <td>
                    <span className={`badge badge-${getSeverityClass(detection.severity)}`}>
                      {detection.severity}
                    </span>
                  </td>
                  <td className="details-cell">
                    {detection.data && (
                      <code className="details-code">
                        {JSON.stringify(detection.data).slice(0, 50)}...
                      </code>
                    )}
                  </td>
                  <td>{formatTimestamp(detection.timestamp)}</td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>

      <div className="pagination">
        <button 
          className="btn btn-secondary"
          disabled={pagination.page <= 1}
          onClick={() => setPagination(p => ({ ...p, page: p.page - 1 }))}
        >
          Previous
        </button>
        <span className="pagination-info">
          Page {pagination.page} of {pagination.totalPages}
        </span>
        <button 
          className="btn btn-secondary"
          disabled={pagination.page >= pagination.totalPages}
          onClick={() => setPagination(p => ({ ...p, page: p.page + 1 }))}
        >
          Next
        </button>
      </div>
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

function formatTimestamp(ts) {
  return new Date(ts).toLocaleString();
}

export default Detections;
