/**
 * Main Application Component
 */

import { useState, useEffect, useMemo } from 'react';
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import api from './services/api';
import { useWebSocket } from './hooks/useWebSocket';
import { PermissionsContext, createPermissionsValue } from './hooks/usePermissions';

// Components
import Sidebar from './components/Sidebar';
import Dashboard from './components/Dashboard';
import Players from './components/Players';
import Detections from './components/Detections';
import Sanctions from './components/Sanctions';
import Rulesets from './components/Rulesets';
import LiveMonitor from './components/LiveMonitor';
import Login from './components/Login';
import ErrorBoundary from './components/ErrorBoundary';
import { ToastProvider } from './components/Toast';
import AdminManagement from './components/AdminManagement';

function App() {
  const [user, setUser] = useState(null);
  const [loading, setLoading] = useState(true);
  const ws = useWebSocket();

  // Create permissions context value
  const permissionsValue = useMemo(
    () => createPermissionsValue(user?.permissions || []),
    [user?.permissions]
  );

  useEffect(() => {
    const token = localStorage.getItem('token');
    if (token) {
      api.getMe()
        .then(setUser)
        .catch(() => {
          localStorage.removeItem('token');
        })
        .finally(() => setLoading(false));
    } else {
      setLoading(false);
    }
  }, []);

  const handleLogin = async (username, password) => {
    const data = await api.login(username, password);
    setUser(data.user);
    return data;
  };

  const handleLogout = () => {
    api.logout();
    setUser(null);
  };

  if (loading) {
    return (
      <div className="loading-screen">
        <div className="loading-spinner"></div>
        <p>Loading...</p>
      </div>
    );
  }

  if (!user) {
    return (
      <ToastProvider>
        <ErrorBoundary message="Failed to load login page">
          <Login onLogin={handleLogin} />
        </ErrorBoundary>
      </ToastProvider>
    );
  }

  return (
    <ToastProvider>
      <PermissionsContext.Provider value={permissionsValue}>
        <Router>
          <div className="app-layout">
            <Sidebar user={user} onLogout={handleLogout} wsConnected={ws.connected} />
            <main className="main-content">
              <ErrorBoundary message="An error occurred in this section">
                <Routes>
                  <Route path="/" element={<Dashboard ws={ws} />} />
                  <Route path="/players" element={<Players />} />
                  <Route path="/detections" element={<Detections />} />
                  <Route path="/sanctions" element={<Sanctions />} />
                  <Route path="/rulesets" element={<Rulesets />} />
                  <Route path="/live" element={<LiveMonitor ws={ws} />} />
                  <Route path="/admin" element={<AdminManagement />} />
                  <Route path="*" element={<Navigate to="/" replace />} />
                </Routes>
              </ErrorBoundary>
            </main>
          </div>
        </Router>
      </PermissionsContext.Provider>
    </ToastProvider>
  );
}

export default App;
