/**
 * Rulesets Management Component
 * Create, edit, duplicate, and activate rulesets with custom detection types
 */

import { useState, useEffect } from 'react';
import api from '../services/api';
import { useToast } from './Toast';
import './Rulesets.css';

// Severity levels
const SEVERITY_LEVELS = ['low', 'medium', 'high', 'critical'];

// Default severity actions
const SEVERITY_ACTIONS = {
  low: 'warn',
  medium: 'warn', 
  high: 'kick',
  critical: 'ban'
};

// Default detection types with their default severities
const DEFAULT_DETECTION_TYPES = {
  speed_hack: { severity: 'high', action: 'kick', enabled: true, description: 'Player moving faster than allowed' },
  teleport: { severity: 'critical', action: 'ban', enabled: true, description: 'Instant position change beyond threshold' },
  godmode: { severity: 'critical', action: 'ban', enabled: true, description: 'Invincibility or health manipulation' },
  health_hack: { severity: 'high', action: 'kick', enabled: true, description: 'Health above maximum allowed' },
  armor_hack: { severity: 'medium', action: 'kick', enabled: true, description: 'Armor above maximum allowed' },
  weapon_blacklist: { severity: 'high', action: 'kick', enabled: true, description: 'Using blacklisted weapon' },
  weapon_ammo: { severity: 'medium', action: 'warn', enabled: true, description: 'Ammo manipulation detected' },
  rapid_fire: { severity: 'high', action: 'kick', enabled: true, description: 'Firing faster than possible' },
  entity_spawn: { severity: 'high', action: 'kick', enabled: true, description: 'Unauthorized entity spawning' },
  entity_blacklist: { severity: 'critical', action: 'ban', enabled: true, description: 'Spawned blacklisted entity' },
  event_blocked: { severity: 'high', action: 'kick', enabled: true, description: 'Triggered blocked server event' },
  event_ratelimit: { severity: 'medium', action: 'warn', enabled: true, description: 'Event rate limit exceeded' },
  resource_injection: { severity: 'critical', action: 'ban', enabled: true, description: 'Injected unauthorized resource' }
};

// Default ruleset template
const DEFAULT_CONFIG = {
  detections: {
    speed: { enabled: true, maxSpeed: 200, maxVehicleSpeed: 500, teleportThreshold: 100 },
    health: { enabled: true, maxHealth: 200, maxArmor: 100, godmodeThreshold: 5 },
    weapons: { enabled: true, blacklistEnabled: true, rapidFireThreshold: 20 },
    entities: { enabled: true, maxPerPlayer: 50, blacklistEnabled: true },
    events: { enabled: true, rateLimit: 30, rateLimitWindow: 1000 },
    resources: { enabled: true, blacklistEnabled: true }
  },
  detection_types: DEFAULT_DETECTION_TYPES,
  sanctions: {
    autoKickThreshold: 3,
    autoBanThreshold: 5,
    defaultBanDuration: 86400
  },
  global_settings: {
    log_level: "info",
    notify_discord: false
  }
};

function Rulesets() {
  const [rulesets, setRulesets] = useState([]);
  const [loading, setLoading] = useState(true);
  const [selected, setSelected] = useState(null);
  const [editing, setEditing] = useState(false);
  const [showCreate, setShowCreate] = useState(false);
  const [activeTab, setActiveTab] = useState('modules'); // 'modules', 'types', 'json'
  const [jsonError, setJsonError] = useState(null);
  const [configText, setConfigText] = useState('');
  const toast = useToast();

  useEffect(() => {
    loadRulesets();
  }, []);

  useEffect(() => {
    if (selected?.config) {
      setConfigText(JSON.stringify(selected.config, null, 2));
      setJsonError(null);
    }
  }, [selected?.id, editing]);

  const loadRulesets = async () => {
    try {
      const data = await api.getRulesets();
      setRulesets(data);
    } catch (err) {
      console.error('Failed to load rulesets:', err);
    } finally {
      setLoading(false);
    }
  };

  const viewRuleset = async (id) => {
    try {
      const data = await api.getRuleset(id);
      // Ensure detection_types exists
      if (!data.config.detection_types) {
        data.config.detection_types = { ...DEFAULT_DETECTION_TYPES };
      }
      setSelected(data);
      setEditing(false);
      setJsonError(null);
    } catch (err) {
      console.error('Failed to load ruleset:', err);
    }
  };

  const activateRuleset = async (id) => {
    try {
      await api.activateRuleset(id);
      await loadRulesets();
      toast.success('Ruleset activated! Changes will sync to FiveM server within 30 seconds.');
    } catch (err) {
      toast.error('Failed to activate ruleset: ' + err.message);
    }
  };

  const saveRuleset = async () => {
    let config = selected.config;
    
    // If on JSON tab, validate and use that
    if (activeTab === 'json') {
      try {
        config = JSON.parse(configText);
      } catch (e) {
        setJsonError('Invalid JSON: ' + e.message);
        return;
      }
    }

    try {
      await api.updateRuleset(selected.id, {
        name: selected.name,
        description: selected.description,
        config: config
      });
      setSelected(s => ({ ...s, config }));
      await loadRulesets();
      setEditing(false);
      setJsonError(null);
      toast.success('Ruleset saved! If active, changes will sync to FiveM server within 30 seconds.');
    } catch (err) {
      toast.error('Failed to save ruleset: ' + err.message);
    }
  };

  const duplicateRuleset = async () => {
    if (!selected) return;
    const newName = prompt('Enter name for duplicate ruleset:', selected.name + ' (Copy)');
    if (!newName) return;
    try {
      await api.createRuleset({ name: newName, description: selected.description, config: selected.config });
      await loadRulesets();
      toast.success('Ruleset duplicated successfully!');
    } catch (err) {
      toast.error('Failed to duplicate ruleset: ' + err.message);
    }
  };

  const deleteRuleset = async () => {
    if (!selected) return;
    if (selected.is_active) {
      toast.warning('Cannot delete the active ruleset. Activate another ruleset first.');
      return;
    }
    if (!confirm(`Delete ruleset "${selected.name}"? This cannot be undone.`)) return;
    try {
      await api.deleteRuleset(selected.id);
      setSelected(null);
      await loadRulesets();
    } catch (err) {
      toast.error('Failed to delete ruleset: ' + err.message);
    }
  };

  const updateDetectionType = (type, field, value) => {
    setSelected(s => ({
      ...s,
      config: {
        ...s.config,
        detection_types: {
          ...s.config.detection_types,
          [type]: {
            ...s.config.detection_types[type],
            [field]: value
          }
        }
      }
    }));
  };

  const addCustomDetection = () => {
    const name = prompt('Enter detection type name (lowercase_with_underscores):');
    if (!name || !/^[a-z_]+$/.test(name)) {
      if (name) toast.warning('Detection name must be lowercase with underscores only');
      return;
    }
    if (selected.config.detection_types[name]) {
      toast.warning('Detection type already exists');
      return;
    }
    setSelected(s => ({
      ...s,
      config: {
        ...s.config,
        detection_types: {
          ...s.config.detection_types,
          [name]: { severity: 'medium', action: 'warn', enabled: true, description: '', custom: true }
        }
      }
    }));
  };

  const removeDetectionType = (type) => {
    if (!confirm(`Remove detection type "${type}"?`)) return;
    setSelected(s => {
      const newTypes = { ...s.config.detection_types };
      delete newTypes[type];
      return { ...s, config: { ...s.config, detection_types: newTypes } };
    });
  };

  const updateModuleConfig = (module, field, value) => {
    setSelected(s => ({
      ...s,
      config: {
        ...s.config,
        detections: {
          ...s.config.detections,
          [module]: {
            ...s.config.detections[module],
            [field]: value
          }
        }
      }
    }));
  };

  const handleConfigChange = (value) => {
    setConfigText(value);
    try {
      JSON.parse(value);
      setJsonError(null);
    } catch (e) {
      setJsonError('Invalid JSON: ' + e.message);
    }
  };

  return (
    <div className="rulesets-page fade-in">
      <div className="page-header">
        <h1>Rulesets</h1>
        <button className="btn btn-primary" onClick={() => setShowCreate(true)}>
          + New Ruleset
        </button>
      </div>

      <div className="sync-notice">
        <span className="sync-icon">🔄</span>
        <span>Rulesets sync to FiveM every 30 seconds. Activate a ruleset to apply it.</span>
      </div>

      <div className="rulesets-layout">
        {/* Rulesets List */}
        <div className="rulesets-list card">
          <div className="card-header">
            <h3 className="card-title">Available Rulesets</h3>
          </div>
          {loading ? (
            <div className="loading">Loading...</div>
          ) : (
            <div className="ruleset-items">
              {rulesets.map(ruleset => (
                <div 
                  key={ruleset.id} 
                  className={`ruleset-item ${selected?.id === ruleset.id ? 'active' : ''}`}
                  onClick={() => viewRuleset(ruleset.id)}
                >
                  <div className="ruleset-info">
                    <span className="ruleset-name">{ruleset.name}</span>
                    <span className="ruleset-version">v{ruleset.version}</span>
                  </div>
                  {ruleset.is_active ? (
                    <span className="badge badge-success">Active</span>
                  ) : (
                    <button 
                      className="btn btn-ghost btn-sm"
                      onClick={(e) => { e.stopPropagation(); activateRuleset(ruleset.id); }}
                    >
                      Activate
                    </button>
                  )}
                </div>
              ))}
            </div>
          )}
        </div>

        {/* Ruleset Editor */}
        <div className="ruleset-editor card">
          {selected ? (
            <>
              <div className="card-header">
                <h3 className="card-title">
                  {editing ? 'Edit Ruleset' : selected.name}
                  {selected.is_active && <span className="badge badge-success ml-sm">Active</span>}
                </h3>
                <div className="flex gap-sm">
                  {editing ? (
                    <>
                      <button className="btn btn-secondary" onClick={() => { setEditing(false); viewRuleset(selected.id); }}>
                        Cancel
                      </button>
                      <button className="btn btn-primary" onClick={saveRuleset} disabled={activeTab === 'json' && !!jsonError}>
                        Save
                      </button>
                    </>
                  ) : (
                    <>
                      <button className="btn btn-ghost" onClick={duplicateRuleset} title="Duplicate">📋</button>
                      <button className="btn btn-ghost" onClick={deleteRuleset} title="Delete" disabled={selected.is_active}>🗑️</button>
                      <button className="btn btn-primary" onClick={() => setEditing(true)}>Edit</button>
                    </>
                  )}
                </div>
              </div>

              {editing && (
                <div className="editor-tabs">
                  <button className={`tab ${activeTab === 'modules' ? 'active' : ''}`} onClick={() => setActiveTab('modules')}>
                    Detection Modules
                  </button>
                  <button className={`tab ${activeTab === 'types' ? 'active' : ''}`} onClick={() => setActiveTab('types')}>
                    Detection Types & Severities
                  </button>
                  <button className={`tab ${activeTab === 'json' ? 'active' : ''}`} onClick={() => { setActiveTab('json'); setConfigText(JSON.stringify(selected.config, null, 2)); }}>
                    Raw JSON
                  </button>
                </div>
              )}

              <div className="editor-content">
                {editing && activeTab === 'modules' && (
                  <ModulesEditor config={selected.config} updateModuleConfig={updateModuleConfig} />
                )}

                {editing && activeTab === 'types' && (
                  <DetectionTypesEditor 
                    types={selected.config.detection_types || {}}
                    updateType={updateDetectionType}
                    addCustom={addCustomDetection}
                    removeType={removeDetectionType}
                  />
                )}

                {editing && activeTab === 'json' && (
                  <div className="input-group">
                    <label className="input-label">
                      Configuration (JSON)
                      {jsonError && <span className="json-error">{jsonError}</span>}
                    </label>
                    <textarea
                      className={`input config-textarea ${jsonError ? 'has-error' : ''}`}
                      value={configText}
                      onChange={(e) => handleConfigChange(e.target.value)}
                      spellCheck="false"
                    />
                  </div>
                )}

                {!editing && (
                  <RulesetSummary config={selected.config} description={selected.description} />
                )}
              </div>
            </>
          ) : (
            <div className="empty-state">
              Select a ruleset to view or edit
            </div>
          )}
        </div>
      </div>

      {showCreate && (
        <CreateRulesetModal 
          onClose={() => setShowCreate(false)}
          onCreated={() => { setShowCreate(false); loadRulesets(); }}
          defaultConfig={DEFAULT_CONFIG}
        />
      )}
    </div>
  );
}

// Modules configuration editor
function ModulesEditor({ config, updateModuleConfig }) {
  const modules = config.detections || {};
  
  return (
    <div className="modules-editor">
      {Object.entries(modules).map(([name, settings]) => (
        <div key={name} className="module-card">
          <div className="module-header">
            <label className="toggle-label">
              <input
                type="checkbox"
                checked={settings.enabled}
                onChange={(e) => updateModuleConfig(name, 'enabled', e.target.checked)}
              />
              <span className="module-name">{name.charAt(0).toUpperCase() + name.slice(1)}</span>
            </label>
          </div>
          {settings.enabled && (
            <div className="module-settings">
              {Object.entries(settings).filter(([k]) => k !== 'enabled').map(([key, value]) => (
                <div key={key} className="setting-row">
                  <label className="setting-label">{key}</label>
                  {typeof value === 'boolean' ? (
                    <input
                      type="checkbox"
                      checked={value}
                      onChange={(e) => updateModuleConfig(name, key, e.target.checked)}
                    />
                  ) : (
                    <input
                      type="number"
                      className="input input-sm"
                      value={value}
                      onChange={(e) => updateModuleConfig(name, key, Number(e.target.value))}
                    />
                  )}
                </div>
              ))}
            </div>
          )}
        </div>
      ))}
    </div>
  );
}

// Detection types and severity editor
function DetectionTypesEditor({ types, updateType, addCustom, removeType }) {
  return (
    <div className="detection-types-editor">
      <div className="types-header">
        <h4>Detection Types & Severities</h4>
        <button className="btn btn-secondary btn-sm" onClick={addCustom}>
          + Add Custom Detection
        </button>
      </div>
      <p className="types-description">
        Configure severity levels and actions for each detection type. Custom detections can be triggered from your FiveM scripts.
      </p>
      
      <div className="types-table">
        <div className="types-header-row">
          <span>Detection Type</span>
          <span>Enabled</span>
          <span>Severity</span>
          <span>Action</span>
          <span></span>
        </div>
        {Object.entries(types).map(([name, config]) => (
          <div key={name} className={`type-row ${config.custom ? 'custom' : ''}`}>
            <div className="type-name">
              <span>{name}</span>
              {config.description && <small>{config.description}</small>}
            </div>
            <div>
              <input
                type="checkbox"
                checked={config.enabled}
                onChange={(e) => updateType(name, 'enabled', e.target.checked)}
              />
            </div>
            <div>
              <select 
                className="input input-sm"
                value={config.severity}
                onChange={(e) => updateType(name, 'severity', e.target.value)}
              >
                {SEVERITY_LEVELS.map(s => (
                  <option key={s} value={s}>{s}</option>
                ))}
              </select>
            </div>
            <div>
              <select
                className="input input-sm"
                value={config.action}
                onChange={(e) => updateType(name, 'action', e.target.value)}
              >
                <option value="warn">Warn</option>
                <option value="kick">Kick</option>
                <option value="ban">Ban</option>
              </select>
            </div>
            <div>
              {config.custom && (
                <button className="btn btn-ghost btn-sm" onClick={() => removeType(name)}>✕</button>
              )}
            </div>
          </div>
        ))}
      </div>
    </div>
  );
}

// Summary view when not editing
function RulesetSummary({ config, description }) {
  const enabledModules = Object.entries(config.detections || {}).filter(([, c]) => c.enabled).length;
  const totalTypes = Object.keys(config.detection_types || {}).length;
  const criticalTypes = Object.values(config.detection_types || {}).filter(t => t.severity === 'critical').length;

  return (
    <div className="ruleset-summary">
      {description && <p className="summary-description">{description}</p>}
      
      <div className="summary-stats">
        <div className="stat-card">
          <span className="stat-value">{enabledModules}</span>
          <span className="stat-label">Active Modules</span>
        </div>
        <div className="stat-card">
          <span className="stat-value">{totalTypes}</span>
          <span className="stat-label">Detection Types</span>
        </div>
        <div className="stat-card">
          <span className="stat-value">{criticalTypes}</span>
          <span className="stat-label">Critical Severity</span>
        </div>
      </div>

      <div className="summary-section">
        <h4>Detection Modules</h4>
        <div className="module-badges">
          {Object.entries(config.detections || {}).map(([name, c]) => (
            <span key={name} className={`badge ${c.enabled ? 'badge-success' : 'badge-secondary'}`}>
              {name}
            </span>
          ))}
        </div>
      </div>

      <div className="summary-section">
        <h4>Sanctions</h4>
        <div className="sanctions-info">
          <span>Auto-kick after <strong>{config.sanctions?.autoKickThreshold || 3}</strong> violations</span>
          <span>Auto-ban after <strong>{config.sanctions?.autoBanThreshold || 5}</strong> violations</span>
        </div>
      </div>
    </div>
  );
}

// Create ruleset modal
function CreateRulesetModal({ onClose, onCreated, defaultConfig }) {
  const [form, setForm] = useState({ name: '', description: '', config: defaultConfig });
  const [loading, setLoading] = useState(false);
  const toast = useToast();

  const handleSubmit = async (e) => {
    e.preventDefault();
    if (!form.name.trim()) { toast.warning('Please enter a ruleset name'); return; }
    setLoading(true);
    try {
      await api.createRuleset(form);
      onCreated();
    } catch (err) {
      toast.error('Failed to create ruleset: ' + err.message);
    } finally {
      setLoading(false);
    }
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal" onClick={e => e.stopPropagation()}>
        <div className="modal-header">
          <h2>Create New Ruleset</h2>
          <button className="btn btn-ghost" onClick={onClose}>✕</button>
        </div>
        <form className="modal-body" onSubmit={handleSubmit}>
          <div className="input-group">
            <label className="input-label">Name *</label>
            <input
              type="text"
              className="input"
              value={form.name}
              onChange={(e) => setForm(f => ({ ...f, name: e.target.value }))}
              placeholder="e.g., Strict Mode, Balanced, Lenient"
              required
              autoFocus
            />
          </div>
          <div className="input-group">
            <label className="input-label">Description</label>
            <input
              type="text"
              className="input"
              value={form.description}
              onChange={(e) => setForm(f => ({ ...f, description: e.target.value }))}
              placeholder="Brief description of this ruleset"
            />
          </div>
          <p className="form-hint">
            The ruleset will be created with default detection types and severities. You can customize them after creation.
          </p>
          <div className="modal-actions">
            <button type="button" className="btn btn-secondary" onClick={onClose}>Cancel</button>
            <button type="submit" className="btn btn-primary" disabled={loading}>
              {loading ? 'Creating...' : 'Create Ruleset'}
            </button>
          </div>
        </form>
      </div>
    </div>
  );
}

export default Rulesets;
