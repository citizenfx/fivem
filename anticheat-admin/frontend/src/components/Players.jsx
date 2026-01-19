/**
 * Players Management Component
 */

import { useState, useEffect } from 'react';
import api from '../services/api';
import './Players.css';

function Players() {
  const [players, setPlayers] = useState([]);
  const [pagination, setPagination] = useState({ page: 1, totalPages: 1 });
  const [search, setSearch] = useState('');
  const [loading, setLoading] = useState(true);
  const [selectedPlayer, setSelectedPlayer] = useState(null);

  useEffect(() => {
    loadPlayers();
  }, [pagination.page, search]);

  const loadPlayers = async () => {
    setLoading(true);
    try {
      const data = await api.getPlayers({ 
        page: pagination.page, 
        search,
        limit: 25 
      });
      setPlayers(data.players);
      setPagination(data.pagination);
    } catch (err) {
      console.error('Failed to load players:', err);
    } finally {
      setLoading(false);
    }
  };

  const handleSearch = (e) => {
    e.preventDefault();
    setPagination(p => ({ ...p, page: 1 }));
  };

  const viewPlayer = async (license) => {
    try {
      const data = await api.getPlayer(license);
      setSelectedPlayer(data);
    } catch (err) {
      console.error('Failed to load player:', err);
    }
  };

  return (
    <div className="players-page fade-in">
      <div className="page-header">
        <h1>Players</h1>
        <form onSubmit={handleSearch} className="search-form">
          <input
            type="text"
            className="input search-input"
            placeholder="Search by name or license..."
            value={search}
            onChange={(e) => setSearch(e.target.value)}
          />
          <button type="submit" className="btn btn-primary">Search</button>
        </form>
      </div>

      <div className="table-container">
        <table className="table">
          <thead>
            <tr>
              <th>Name</th>
              <th>License</th>
              <th>First Seen</th>
              <th>Last Seen</th>
              <th>Detections</th>
              <th>Trust</th>
              <th>Actions</th>
            </tr>
          </thead>
          <tbody>
            {loading ? (
              <tr><td colSpan="7" className="text-center">Loading...</td></tr>
            ) : players.length === 0 ? (
              <tr><td colSpan="7" className="text-center">No players found</td></tr>
            ) : (
              players.map(player => (
                <tr key={player.license}>
                  <td><strong>{player.name}</strong></td>
                  <td className="license-cell">{player.license}</td>
                  <td>{formatDate(player.first_seen)}</td>
                  <td>{formatDate(player.last_seen)}</td>
                  <td>
                    <span className={`badge ${player.total_detections > 0 ? 'badge-warning' : 'badge-success'}`}>
                      {player.total_detections}
                    </span>
                  </td>
                  <td>{getTrustBadge(player.trust_level)}</td>
                  <td>
                    <button 
                      className="btn btn-ghost btn-sm"
                      onClick={() => viewPlayer(player.license)}
                    >
                      View
                    </button>
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>

      {/* Pagination */}
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

      {/* Player Detail Modal */}
      {selectedPlayer && (
        <PlayerModal 
          player={selectedPlayer} 
          onClose={() => setSelectedPlayer(null)} 
        />
      )}
    </div>
  );
}

function PlayerModal({ player, onClose }) {
  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal" onClick={e => e.stopPropagation()}>
        <div className="modal-header">
          <h2>{player.name}</h2>
          <button className="btn btn-ghost" onClick={onClose}>✕</button>
        </div>
        <div className="modal-body">
          <div className="player-details">
            <div className="detail-row">
              <span className="detail-label">License:</span>
              <span className="detail-value">{player.license}</span>
            </div>
            <div className="detail-row">
              <span className="detail-label">Steam ID:</span>
              <span className="detail-value">{player.steam_id || 'N/A'}</span>
            </div>
            <div className="detail-row">
              <span className="detail-label">First Seen:</span>
              <span className="detail-value">{formatDate(player.first_seen)}</span>
            </div>
            <div className="detail-row">
              <span className="detail-label">Total Detections:</span>
              <span className="detail-value">{player.total_detections}</span>
            </div>
          </div>

          {player.detections?.length > 0 && (
            <div className="player-section">
              <h3>Recent Detections</h3>
              <div className="mini-list">
                {player.detections.slice(0, 10).map(d => (
                  <div key={d.id} className="mini-item">
                    <span>{d.detection_type}</span>
                    <span className="text-muted">{formatDate(d.timestamp)}</span>
                  </div>
                ))}
              </div>
            </div>
          )}

          {player.sanctions?.length > 0 && (
            <div className="player-section">
              <h3>Sanctions</h3>
              <div className="mini-list">
                {player.sanctions.map(s => (
                  <div key={s.id} className="mini-item">
                    <span className={`badge badge-${s.sanction_type === 'ban' ? 'error' : 'warning'}`}>
                      {s.sanction_type}
                    </span>
                    <span>{s.reason}</span>
                  </div>
                ))}
              </div>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}

function formatDate(dateStr) {
  if (!dateStr) return 'N/A';
  return new Date(dateStr).toLocaleDateString();
}

function getTrustBadge(level) {
  if (level >= 10) return <span className="badge badge-success">Trusted</span>;
  if (level < 0) return <span className="badge badge-error">Flagged</span>;
  return <span className="badge badge-info">Normal</span>;
}

export default Players;
