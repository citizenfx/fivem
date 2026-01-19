/**
 * Sanctions Management Component
 */

import { useState, useEffect } from 'react';
import api from '../services/api';
import { useToast } from './Toast';
import './Sanctions.css';

function Sanctions() {
  const [sanctions, setSanctions] = useState([]);
  const [pagination, setPagination] = useState({ page: 1, totalPages: 1 });
  const [loading, setLoading] = useState(true);
  const [showCreate, setShowCreate] = useState(false);
  const toast = useToast();

  useEffect(() => {
    loadSanctions();
  }, [pagination.page]);

  const loadSanctions = async () => {
    setLoading(true);
    try {
      const data = await api.getSanctions({ page: pagination.page, limit: 25 });
      setSanctions(data.sanctions);
      setPagination(data.pagination);
    } catch (err) {
      console.error('Failed to load sanctions:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleLiftBan = async (id) => {
    if (!confirm('Are you sure you want to lift this sanction?')) return;
    try {
      await api.updateSanction(id, { is_active: false });
      loadSanctions();
      toast.success('Sanction lifted successfully');
    } catch (err) {
      toast.error('Failed to lift sanction: ' + err.message);
    }
  };

  return (
    <div className="sanctions-page fade-in">
      <div className="page-header">
        <h1>Sanctions</h1>
        <button className="btn btn-primary" onClick={() => setShowCreate(true)}>
          + New Sanction
        </button>
      </div>

      <div className="table-container">
        <table className="table">
          <thead>
            <tr>
              <th>ID</th>
              <th>Player</th>
              <th>Type</th>
              <th>Reason</th>
              <th>Status</th>
              <th>Created</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan="7" className="text-center">Loading...</td></tr>
            ) : sanctions.length === 0 ? (
              <tr><td colSpan="7" className="text-center">No sanctions found</td></tr>
            ) : (
              sanctions.map(sanction => (
                <tr key={sanction.id}>
                  <td>#{sanction.id}</td>
                  <td><strong>{sanction.player_name}</strong></td>
                  <td>
                    <span className={`badge badge-${getSanctionClass(sanction.sanction_type)}`}>
                      {sanction.sanction_type}
                    </span>
                  </td>
                  <td className="reason-cell">{sanction.reason}</td>
                  <td>
                    {sanction.is_active ? (
                      <span className="badge badge-error">Active</span>
                    ) : (
                      <span className="badge badge-success">Lifted</span>
                    )}
                  </td>
                  <td>{formatDate(sanction.created_at)}</td>
                  <td>
                    {sanction.is_active && sanction.sanction_type === 'ban' && (
                      <button 
                        className="btn btn-ghost btn-sm"
                        onClick={() => handleLiftBan(sanction.id)}
                      >
                        Lift
                      </button>
                    )}
                  </td>
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

      {showCreate && (
        <CreateSanctionModal 
          onClose={() => setShowCreate(false)}
          onCreated={() => { setShowCreate(false); loadSanctions(); }}
        />
      )}
    </div>
  );
}

function CreateSanctionModal({ onClose, onCreated }) {
  const [form, setForm] = useState({
    player_license: '',
    player_name: '',
    sanction_type: 'ban',
    reason: '',
    duration: 86400
  });
  const [loading, setLoading] = useState(false);
  const toast = useToast();

  const handleSubmit = async (e) => {
    e.preventDefault();
    setLoading(true);
    try {
      await api.createSanction(form);
      onCreated();
      toast.success('Sanction created successfully');
    } catch (err) {
      toast.error('Failed to create sanction: ' + err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal" onClick={e => e.stopPropagation()}>
        <div className="modal-header">
          <h2>Create Sanction</h2>
          <button className="btn btn-ghost" onClick={onClose}>✕</button>
        </div>
        <form className="modal-body" onSubmit={handleSubmit}>
          <div className="input-group">
            <label className="input-label">Player License</label>
            <input
              type="text"
              className="input"
              value={form.player_license}
              onChange={(e) => setForm(f => ({ ...f, player_license: e.target.value }))}
              placeholder="license:12345..."
              required
            />
          </div>
          <div className="input-group">
            <label className="input-label">Player Name</label>
            <input
              type="text"
              className="input"
              value={form.player_name}
              onChange={(e) => setForm(f => ({ ...f, player_name: e.target.value }))}
              placeholder="Player name"
            />
          </div>
          <div className="input-group">
            <label className="input-label">Type</label>
            <select
              className="input"
              value={form.sanction_type}
              onChange={(e) => setForm(f => ({ ...f, sanction_type: e.target.value }))}
            >
              <option value="warn">Warn</option>
              <option value="kick">Kick</option>
              <option value="ban">Ban</option>
            </select>
          </div>
          <div className="input-group">
            <label className="input-label">Reason</label>
            <input
              type="text"
              className="input"
              value={form.reason}
              onChange={(e) => setForm(f => ({ ...f, reason: e.target.value }))}
              placeholder="Reason for sanction"
              required
            />
          </div>
          {form.sanction_type === 'ban' && (
            <div className="input-group">
              <label className="input-label">Duration</label>
              <select
                className="input"
                value={form.duration}
                onChange={(e) => setForm(f => ({ ...f, duration: parseInt(e.target.value) }))}
              >
                <option value={3600}>1 Hour</option>
                <option value={86400}>1 Day</option>
                <option value={604800}>1 Week</option>
                <option value={2592000}>30 Days</option>
                <option value={0}>Permanent</option>
              </select>
            </div>
          )}
          <button type="submit" className="btn btn-primary" disabled={loading}>
            {loading ? 'Creating...' : 'Create Sanction'}
          </button>
        </form>
      </div>
    </div>
  );
}

function getSanctionClass(type) {
  switch (type) {
    case 'warn': return 'warning';
    case 'kick': return 'warning';
    case 'ban': return 'error';
    default: return 'info';
  }
}

function formatDate(dateStr) {
  return new Date(dateStr).toLocaleString();
}

export default Sanctions;
