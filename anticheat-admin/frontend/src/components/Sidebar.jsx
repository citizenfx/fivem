/**
 * Sidebar Navigation Component
 */

import { NavLink } from 'react-router-dom';
import { usePermissions, NAV_ITEMS } from '../hooks/usePermissions';
import './Sidebar.css';

function Sidebar({ user, onLogout, wsConnected }) {
  const { hasPermission } = usePermissions();

  // Filter nav items based on user permissions
  const visibleNavItems = NAV_ITEMS.filter(item =>
    !item.permission || hasPermission(item.permission)
  );

  return (
    <aside className="sidebar">
      <div className="sidebar-header">
        <div className="logo">
          <span className="logo-icon">🛡️</span>
          <span className="logo-text">Anticheat</span>
        </div>
      </div>

      <nav className="sidebar-nav">
        {visibleNavItems.map(item => (
          <NavLink
            key={item.path}
            to={item.path}
            className={({ isActive }) =>
              `nav-item ${isActive ? 'active' : ''}`
            }
          >
            <span className="nav-icon">{item.icon}</span>
            <span className="nav-label">{item.label}</span>
            {item.path === '/live' && wsConnected && (
              <span className="live-dot"></span>
            )}
          </NavLink>
        ))}
      </nav>

      <div className="sidebar-footer">
        <div className="connection-status">
          <span className={`status-dot ${wsConnected ? 'connected' : 'disconnected'}`}></span>
          <span className="status-text">
            {wsConnected ? 'Connected' : 'Disconnected'}
          </span>
        </div>
        
        <div className="user-info">
          <div className="user-avatar">
            {user?.username?.charAt(0).toUpperCase()}
          </div>
          <div className="user-details">
            <span className="user-name">{user?.username}</span>
            <span className="user-role">{user?.role}</span>
          </div>
          <button className="logout-btn" onClick={onLogout} title="Logout">
            ⏻
          </button>
        </div>
      </div>
    </aside>
  );
}

export default Sidebar;
