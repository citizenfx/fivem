/**
 * Admin Management Component
 * Manage users, roles, and groups
 */

import { useState, useEffect } from 'react';
import api from '../services/api';
import { useToast } from './Toast';
import { usePermissions, RequirePermission } from '../hooks/usePermissions';
import './AdminManagement.css';

function AdminManagement() {
  const [activeTab, setActiveTab] = useState('users');
  const [users, setUsers] = useState([]);
  const [roles, setRoles] = useState([]);
  const [groups, setGroups] = useState([]);
  const [permissions, setPermissions] = useState({});
  const [loading, setLoading] = useState(true);
  const [showModal, setShowModal] = useState(null);
  const [editItem, setEditItem] = useState(null);
  const { showToast } = useToast();
  const { hasPermission } = usePermissions();

  useEffect(() => {
    loadData();
  }, []);

  const loadData = async () => {
    setLoading(true);
    try {
      const [usersRes, rolesRes, groupsRes, permsRes] = await Promise.all([
        hasPermission('admin.users.view') ? api.get('/admin/users') : Promise.resolve([]),
        hasPermission('admin.roles.view') ? api.get('/admin/roles') : Promise.resolve([]),
        hasPermission('admin.groups.view') ? api.get('/admin/groups') : Promise.resolve([]),
        hasPermission('admin.roles.view') ? api.get('/admin/permissions') : Promise.resolve({}),
      ]);
      setUsers(usersRes);
      setRoles(rolesRes);
      setGroups(groupsRes);
      setPermissions(permsRes);
    } catch (error) {
      showToast('Failed to load admin data', 'error');
    } finally {
      setLoading(false);
    }
  };

  const handleCreateUser = async (userData) => {
    try {
      await api.post('/admin/users', userData);
      showToast('User created successfully', 'success');
      setShowModal(null);
      loadData();
    } catch (error) {
      showToast(error.message || 'Failed to create user', 'error');
    }
  };

  const handleUpdateUser = async (id, userData) => {
    try {
      await api.patch(`/admin/users/${id}`, userData);
      showToast('User updated successfully', 'success');
      setShowModal(null);
      setEditItem(null);
      loadData();
    } catch (error) {
      showToast(error.message || 'Failed to update user', 'error');
    }
  };

  const handleDeleteUser = async (id) => {
    if (!confirm('Are you sure you want to deactivate this user?')) return;
    try {
      await api.delete(`/admin/users/${id}`);
      showToast('User deactivated', 'success');
      loadData();
    } catch (error) {
      showToast(error.message || 'Failed to delete user', 'error');
    }
  };

  if (loading) {
    return <div className="admin-loading">Loading...</div>;
  }

  return (
    <div className="admin-management">
      <header className="admin-header">
        <h1>Admin Management</h1>
      </header>

      <div className="admin-tabs">
        <RequirePermission permission="admin.users.view">
          <button 
            className={`tab ${activeTab === 'users' ? 'active' : ''}`}
            onClick={() => setActiveTab('users')}
          >
            Users ({users.length})
          </button>
        </RequirePermission>
        <RequirePermission permission="admin.roles.view">
          <button 
            className={`tab ${activeTab === 'roles' ? 'active' : ''}`}
            onClick={() => setActiveTab('roles')}
          >
            Roles ({roles.length})
          </button>
        </RequirePermission>
        <RequirePermission permission="admin.groups.view">
          <button 
            className={`tab ${activeTab === 'groups' ? 'active' : ''}`}
            onClick={() => setActiveTab('groups')}
          >
            Groups ({groups.length})
          </button>
        </RequirePermission>
      </div>

      <div className="admin-content">
        {activeTab === 'users' && (
          <UsersTab 
            users={users} 
            roles={roles}
            groups={groups}
            onEdit={(user) => { setEditItem(user); setShowModal('editUser'); }}
            onDelete={handleDeleteUser}
            onCreate={() => setShowModal('createUser')}
          />
        )}
        {activeTab === 'roles' && (
          <RolesTab 
            roles={roles}
            permissions={permissions}
            onReload={loadData}
          />
        )}
        {activeTab === 'groups' && (
          <GroupsTab 
            groups={groups}
            onReload={loadData}
          />
        )}
      </div>

      {showModal === 'createUser' && (
        <UserModal 
          roles={roles}
          groups={groups}
          onSave={handleCreateUser}
          onClose={() => setShowModal(null)}
        />
      )}
      {showModal === 'editUser' && editItem && (
        <UserModal 
          user={editItem}
          roles={roles}
          groups={groups}
          onSave={(data) => handleUpdateUser(editItem.id, data)}
          onClose={() => { setShowModal(null); setEditItem(null); }}
        />
      )}
    </div>
  );
}

// ==================== SUB-COMPONENTS ====================

function UsersTab({ users, roles, groups, onEdit, onDelete, onCreate }) {
  const { hasPermission } = usePermissions();

  return (
    <div className="users-tab">
      <div className="tab-header">
        <h2>Admin Users</h2>
        {hasPermission('admin.users.manage') && (
          <button className="btn-primary" onClick={onCreate}>+ Add User</button>
        )}
      </div>
      <table className="admin-table">
        <thead>
          <tr>
            <th>Username</th>
            <th>Email</th>
            <th>Role</th>
            <th>Group</th>
            <th>Status</th>
            <th>Last Login</th>
            <th>Actions</th>
          </tr>
        </thead>
        <tbody>
          {users.map(user => (
            <tr key={user.id}>
              <td>{user.username}</td>
              <td>{user.email || '-'}</td>
              <td><span className="badge role">{user.role_name || 'None'}</span></td>
              <td>{user.group_name || '-'}</td>
              <td>
                <span className={`badge ${user.is_active ? 'active' : 'inactive'}`}>
                  {user.is_active ? 'Active' : 'Inactive'}
                </span>
              </td>
              <td>{user.last_login ? new Date(user.last_login).toLocaleString() : 'Never'}</td>
              <td>
                {hasPermission('admin.users.manage') && (
                  <>
                    <button className="btn-sm" onClick={() => onEdit(user)}>Edit</button>
                    <button className="btn-sm btn-danger" onClick={() => onDelete(user.id)}>Delete</button>
                  </>
                )}
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

function RolesTab({ roles, permissions, onReload }) {
  const { hasPermission } = usePermissions();
  const { showToast } = useToast();
  const [expandedRole, setExpandedRole] = useState(null);
  const [showModal, setShowModal] = useState(false);
  const [editRole, setEditRole] = useState(null);

  const handleDeleteRole = async (id) => {
    if (!confirm('Are you sure you want to delete this role?')) return;
    try {
      await api.delete(`/admin/roles/${id}`);
      showToast('Role deleted', 'success');
      onReload();
    } catch (error) {
      showToast(error.message || 'Failed to delete role', 'error');
    }
  };

  const handleCreateRole = async (roleData) => {
    try {
      await api.post('/admin/roles', roleData);
      showToast('Role created successfully', 'success');
      setShowModal(false);
      onReload();
    } catch (error) {
      showToast(error.message || 'Failed to create role', 'error');
    }
  };

  const handleUpdateRole = async (id, roleData) => {
    try {
      await api.put(`/admin/roles/${id}`, roleData);
      showToast('Role updated successfully', 'success');
      setShowModal(false);
      setEditRole(null);
      onReload();
    } catch (error) {
      showToast(error.message || 'Failed to update role', 'error');
    }
  };

  const handleEditClick = (role) => {
    setEditRole(role);
    setShowModal(true);
  };

  return (
    <div className="roles-tab">
      <div className="tab-header">
        <h2>Roles</h2>
        {hasPermission('admin.roles.manage') && (
          <button className="btn-primary" onClick={() => { setEditRole(null); setShowModal(true); }}>
            + Create Role
          </button>
        )}
      </div>
      <div className="roles-list">
        {roles.map(role => (
          <div key={role.id} className="role-card">
            <div className="role-header" onClick={() => setExpandedRole(expandedRole === role.id ? null : role.id)}>
              <div className="role-info">
                <span className="role-name">{role.name}</span>
                {role.is_system ? <span className="badge system">System</span> : null}
              </div>
              <span className="expand-icon">{expandedRole === role.id ? '▼' : '▶'}</span>
            </div>
            {expandedRole === role.id && (
              <div className="role-details">
                <p className="role-description">{role.description || 'No description'}</p>
                <div className="role-permissions">
                  <strong>Permissions ({role.permissions?.length || 0}):</strong>
                  <div className="permission-tags">
                    {(role.permissions || []).map(p => (
                      <span key={p} className="permission-tag">{p}</span>
                    ))}
                  </div>
                </div>
                {hasPermission('admin.roles.manage') && (
                  <div className="role-actions">
                    {!role.is_system && (
                      <>
                        <button className="btn-sm" onClick={() => handleEditClick(role)}>
                          Edit Role
                        </button>
                        <button className="btn-sm btn-danger" onClick={() => handleDeleteRole(role.id)}>
                          Delete Role
                        </button>
                      </>
                    )}
                  </div>
                )}
              </div>
            )}
          </div>
        ))}
      </div>

      {showModal && (
        <RoleModal
          role={editRole}
          permissions={permissions}
          onSave={(data) => editRole ? handleUpdateRole(editRole.id, data) : handleCreateRole(data)}
          onClose={() => { setShowModal(false); setEditRole(null); }}
        />
      )}
    </div>
  );
}

function RoleModal({ role, permissions, onSave, onClose }) {
  const [formData, setFormData] = useState({
    name: role?.name || '',
    description: role?.description || '',
    permissions: role?.permissions || [],
  });

  // Group permissions by category
  const permissionsByCategory = {};
  if (permissions && typeof permissions === 'object') {
    Object.entries(permissions).forEach(([name, info]) => {
      const category = info.category || 'other';
      if (!permissionsByCategory[category]) {
        permissionsByCategory[category] = [];
      }
      permissionsByCategory[category].push({ name, ...info });
    });
  }

  const togglePermission = (permName) => {
    setFormData(prev => ({
      ...prev,
      permissions: prev.permissions.includes(permName)
        ? prev.permissions.filter(p => p !== permName)
        : [...prev.permissions, permName]
    }));
  };

  const toggleCategory = (category) => {
    const categoryPerms = permissionsByCategory[category].map(p => p.name);
    const allSelected = categoryPerms.every(p => formData.permissions.includes(p));

    if (allSelected) {
      setFormData(prev => ({
        ...prev,
        permissions: prev.permissions.filter(p => !categoryPerms.includes(p))
      }));
    } else {
      setFormData(prev => ({
        ...prev,
        permissions: [...new Set([...prev.permissions, ...categoryPerms])]
      }));
    }
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    onSave(formData);
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal modal-large" onClick={(e) => e.stopPropagation()}>
        <h2>{role ? 'Edit Role' : 'Create Role'}</h2>
        <form onSubmit={handleSubmit}>
          <div className="form-group">
            <label>Role Name</label>
            <input
              type="text"
              value={formData.name}
              onChange={(e) => setFormData({...formData, name: e.target.value})}
              required
              disabled={!!role}
              placeholder="e.g., custom_moderator"
            />
          </div>
          <div className="form-group">
            <label>Description</label>
            <input
              type="text"
              value={formData.description}
              onChange={(e) => setFormData({...formData, description: e.target.value})}
              placeholder="Role description"
            />
          </div>

          <div className="permissions-section">
            <label>Permissions</label>
            <div className="permissions-grid">
              {Object.entries(permissionsByCategory).map(([category, perms]) => {
                const allSelected = perms.every(p => formData.permissions.includes(p.name));
                const someSelected = perms.some(p => formData.permissions.includes(p.name));

                return (
                  <div key={category} className="permission-category">
                    <div className="category-header">
                      <label className="checkbox-label">
                        <input
                          type="checkbox"
                          checked={allSelected}
                          ref={(el) => { if (el) el.indeterminate = someSelected && !allSelected; }}
                          onChange={() => toggleCategory(category)}
                        />
                        <span className="category-name">{category.charAt(0).toUpperCase() + category.slice(1)}</span>
                      </label>
                    </div>
                    <div className="category-permissions">
                      {perms.map(perm => (
                        <label key={perm.name} className="permission-checkbox">
                          <input
                            type="checkbox"
                            checked={formData.permissions.includes(perm.name)}
                            onChange={() => togglePermission(perm.name)}
                          />
                          <span className="perm-name">{perm.name}</span>
                          <span className="perm-desc">{perm.description}</span>
                        </label>
                      ))}
                    </div>
                  </div>
                );
              })}
            </div>
          </div>

          <div className="modal-actions">
            <button type="button" className="btn-secondary" onClick={onClose}>Cancel</button>
            <button type="submit" className="btn-primary">{role ? 'Update Role' : 'Create Role'}</button>
          </div>
        </form>
      </div>
    </div>
  );
}

function GroupsTab({ groups, onReload }) {
  const { hasPermission } = usePermissions();
  const { showToast } = useToast();
  const [showModal, setShowModal] = useState(false);
  const [editGroup, setEditGroup] = useState(null);

  const handleCreateGroup = async (groupData) => {
    try {
      await api.post('/admin/groups', groupData);
      showToast('Group created', 'success');
      setShowModal(false);
      onReload();
    } catch (error) {
      showToast(error.message || 'Failed to create group', 'error');
    }
  };

  const handleUpdateGroup = async (id, groupData) => {
    try {
      await api.put(`/admin/groups/${id}`, groupData);
      showToast('Group updated', 'success');
      setShowModal(false);
      setEditGroup(null);
      onReload();
    } catch (error) {
      showToast(error.message || 'Failed to update group', 'error');
    }
  };

  const handleDeleteGroup = async (id) => {
    if (!confirm('Are you sure you want to delete this group?')) return;
    try {
      await api.delete(`/admin/groups/${id}`);
      showToast('Group deleted', 'success');
      onReload();
    } catch (error) {
      showToast(error.message || 'Failed to delete group', 'error');
    }
  };

  return (
    <div className="groups-tab">
      <div className="tab-header">
        <h2>Groups</h2>
        {hasPermission('admin.groups.manage') && (
          <button className="btn-primary" onClick={() => { setEditGroup(null); setShowModal(true); }}>
            + Create Group
          </button>
        )}
      </div>
      <div className="groups-list">
        {groups.length === 0 && (
          <div className="empty-state">No groups created yet</div>
        )}
        {groups.map(group => (
          <div key={group.id} className="group-card">
            <div className="group-info">
              <span className="group-name">{group.name}</span>
              <span className="group-description">{group.description || 'No description'}</span>
            </div>
            {hasPermission('admin.groups.manage') && (
              <div className="group-actions">
                <button className="btn-sm" onClick={() => { setEditGroup(group); setShowModal(true); }}>
                  Edit
                </button>
                <button className="btn-sm btn-danger" onClick={() => handleDeleteGroup(group.id)}>
                  Delete
                </button>
              </div>
            )}
          </div>
        ))}
      </div>

      {showModal && (
        <GroupModal
          group={editGroup}
          onSave={(data) => editGroup ? handleUpdateGroup(editGroup.id, data) : handleCreateGroup(data)}
          onClose={() => { setShowModal(false); setEditGroup(null); }}
        />
      )}
    </div>
  );
}

function GroupModal({ group, onSave, onClose }) {
  const [formData, setFormData] = useState({
    name: group?.name || '',
    description: group?.description || '',
  });

  const handleSubmit = (e) => {
    e.preventDefault();
    onSave(formData);
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal" onClick={(e) => e.stopPropagation()}>
        <h2>{group ? 'Edit Group' : 'Create Group'}</h2>
        <form onSubmit={handleSubmit}>
          <div className="form-group">
            <label>Group Name</label>
            <input
              type="text"
              value={formData.name}
              onChange={(e) => setFormData({...formData, name: e.target.value})}
              required
              placeholder="e.g., Server Team A"
            />
          </div>
          <div className="form-group">
            <label>Description</label>
            <textarea
              value={formData.description}
              onChange={(e) => setFormData({...formData, description: e.target.value})}
              placeholder="Group description"
              rows={3}
            />
          </div>
          <div className="modal-actions">
            <button type="button" className="btn-secondary" onClick={onClose}>Cancel</button>
            <button type="submit" className="btn-primary">{group ? 'Update Group' : 'Create Group'}</button>
          </div>
        </form>
      </div>
    </div>
  );
}

function UserModal({ user, roles, groups, onSave, onClose }) {
  const [formData, setFormData] = useState({
    username: user?.username || '',
    email: user?.email || '',
    password: '',
    role_id: user?.role_id || '',
    group_id: user?.group_id || '',
    is_active: user?.is_active !== false,
  });

  const handleSubmit = (e) => {
    e.preventDefault();
    const data = { ...formData };
    if (!data.password) delete data.password;
    if (!data.role_id) data.role_id = null;
    if (!data.group_id) data.group_id = null;
    onSave(data);
  };

  return (
    <div className="modal-overlay" onClick={onClose}>
      <div className="modal" onClick={(e) => e.stopPropagation()}>
        <h2>{user ? 'Edit User' : 'Create User'}</h2>
        <form onSubmit={handleSubmit}>
          <div className="form-group">
            <label>Username</label>
            <input
              type="text"
              value={formData.username}
              onChange={(e) => setFormData({...formData, username: e.target.value})}
              required
              disabled={!!user}
            />
          </div>
          <div className="form-group">
            <label>Email</label>
            <input
              type="email"
              value={formData.email}
              onChange={(e) => setFormData({...formData, email: e.target.value})}
            />
          </div>
          <div className="form-group">
            <label>{user ? 'New Password (leave blank to keep)' : 'Password'}</label>
            <input
              type="password"
              value={formData.password}
              onChange={(e) => setFormData({...formData, password: e.target.value})}
              required={!user}
            />
          </div>
          <div className="form-group">
            <label>Role</label>
            <select
              value={formData.role_id}
              onChange={(e) => setFormData({...formData, role_id: e.target.value})}
            >
              <option value="">No Role</option>
              {roles.map(r => <option key={r.id} value={r.id}>{r.name}</option>)}
            </select>
          </div>
          <div className="form-group">
            <label>Group</label>
            <select
              value={formData.group_id}
              onChange={(e) => setFormData({...formData, group_id: e.target.value})}
            >
              <option value="">No Group</option>
              {groups.map(g => <option key={g.id} value={g.id}>{g.name}</option>)}
            </select>
          </div>
          {user && (
            <div className="form-group checkbox">
              <label>
                <input
                  type="checkbox"
                  checked={formData.is_active}
                  onChange={(e) => setFormData({...formData, is_active: e.target.checked})}
                />
                Active
              </label>
            </div>
          )}
          <div className="modal-actions">
            <button type="button" className="btn-secondary" onClick={onClose}>Cancel</button>
            <button type="submit" className="btn-primary">{user ? 'Update' : 'Create'}</button>
          </div>
        </form>
      </div>
    </div>
  );
}

export default AdminManagement;

