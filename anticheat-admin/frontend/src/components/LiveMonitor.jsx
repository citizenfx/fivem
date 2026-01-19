/**
 * Live Monitor Component
 * Real-time WebSocket feed of detections and sanctions
 */

import { useState, useEffect, useRef } from 'react';
import './LiveMonitor.css';

function LiveMonitor({ ws }) {
  const [events, setEvents] = useState([]);
  const feedRef = useRef(null);
  const maxEvents = 100;

  useEffect(() => {
    // Subscribe to all events
    const unsubDetection = ws.subscribe('detection', (data) => {
      addEvent('detection', data);
    });

    const unsubSanction = ws.subscribe('sanction', (data) => {
      addEvent('sanction', data);
    });

    const unsubConnect = ws.subscribe('player_connect', (data) => {
      addEvent('connect', data);
    });

    const unsubDisconnect = ws.subscribe('player_disconnect', (data) => {
      addEvent('disconnect', data);
    });

    return () => {
      unsubDetection();
      unsubSanction();
      unsubConnect();
      unsubDisconnect();
    };
  }, [ws]);

  const addEvent = (type, data) => {
    const event = {
      id: Date.now() + Math.random(),
      type,
      data,
      timestamp: new Date().toISOString()
    };

    setEvents(prev => {
      const updated = [event, ...prev];
      if (updated.length > maxEvents) {
        updated.pop();
      }
      return updated;
    });
  };

  const clearEvents = () => setEvents([]);

  return (
    <div className="live-monitor fade-in">
      <div className="page-header">
        <div className="flex items-center gap-md">
          <h1>Live Monitor</h1>
          <span className={`connection-badge ${ws.connected ? 'connected' : 'disconnected'}`}>
            {ws.connected ? '● Connected' : '○ Disconnected'}
          </span>
        </div>
        <button className="btn btn-secondary" onClick={clearEvents}>
          Clear
        </button>
      </div>

      <div className="monitor-card card">
        <div className="card-header">
          <h3 className="card-title">Event Feed</h3>
          <span className="event-count">{events.length} events</span>
        </div>

        <div className="event-feed" ref={feedRef}>
          {events.length === 0 ? (
            <div className="empty-state">
              Waiting for events...
              <br />
              <span className="text-muted">Events will appear here in real-time</span>
            </div>
          ) : (
            events.map(event => (
              <EventItem key={event.id} event={event} />
            ))
          )}
        </div>
      </div>
    </div>
  );
}

function EventItem({ event }) {
  const { type, data, timestamp } = event;

  const getIcon = () => {
    switch (type) {
      case 'detection': return '🔍';
      case 'sanction': return '🔨';
      case 'connect': return '🟢';
      case 'disconnect': return '🔴';
      default: return '📌';
    }
  };

  const getTypeClass = () => {
    switch (type) {
      case 'detection': return 'event-detection';
      case 'sanction': return 'event-sanction';
      case 'connect': return 'event-connect';
      case 'disconnect': return 'event-disconnect';
      default: return '';
    }
  };

  const getTitle = () => {
    switch (type) {
      case 'detection':
        return `Detection: ${data.detection_type}`;
      case 'sanction':
        return `${data.sanction_type?.toUpperCase()}: ${data.reason}`;
      case 'connect':
        return `Player Connected: ${data.name || data.license}`;
      case 'disconnect':
        return `Player Disconnected: ${data.license}`;
      default:
        return 'Event';
    }
  };

  const getPlayer = () => {
    return data.player_name || data.name || data.license || 'Unknown';
  };

  return (
    <div className={`event-item ${getTypeClass()}`}>
      <span className="event-icon">{getIcon()}</span>
      <div className="event-content">
        <div className="event-title">{getTitle()}</div>
        <div className="event-meta">
          <span className="event-player">{getPlayer()}</span>
          <span className="event-time">{formatTime(timestamp)}</span>
        </div>
      </div>
      {type === 'detection' && data.severity && (
        <span className={`badge badge-${getSeverityClass(data.severity)}`}>
          {data.severity}
        </span>
      )}
    </div>
  );
}

function formatTime(timestamp) {
  return new Date(timestamp).toLocaleTimeString();
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

export default LiveMonitor;
