/**
 * Admin Management Routes
 * CRUD for users, roles, groups, and permissions
 */

const express = require('express');
const bcrypt = require('bcrypt');
const db = require('../db');
const { authenticateToken, requirePermission, clearPermissionCache } = require('../middleware/auth');
const { PERMISSIONS, DEFAULT_ROLES } = require('../services/permissions');
const logger = require('../services/logger');

const router = express.Router();

// All routes require authentication
router.use(authenticateToken);

// ==================== USERS ====================

/**
 * GET /api/admin/users
 * List all admin users
 */
router.get('/users', requirePermission('admin.users.view'), async (req, res) => {
    try {
        const users = await db.query(`
            SELECT a.id, a.username, a.email, a.role_id, a.group_id, a.is_active,
                   a.last_login, a.created_at, r.name as role_name, g.name as group_name
            FROM admins a
            LEFT JOIN roles r ON a.role_id = r.id
            LEFT JOIN \`groups\` g ON a.group_id = g.id
            ORDER BY a.created_at DESC
        `);
        res.json(users || []);
    } catch (error) {
        logger.error('List users error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch users' });
    }
});

/**
 * POST /api/admin/users
 * Create new admin user
 */
router.post('/users', requirePermission('admin.users.manage'), async (req, res) => {
    try {
        const { username, email, password, role_id, group_id } = req.body;

        if (!username || !password) {
            return res.status(400).json({ error: 'Username and password required' });
        }

        const existing = await db.queryOne('SELECT id FROM admins WHERE username = ?', [username]);
        if (existing) {
            return res.status(409).json({ error: 'Username already exists' });
        }

        const passwordHash = await bcrypt.hash(password, 10);
        const result = await db.run(`
            INSERT INTO admins (username, email, password_hash, role_id, group_id)
            VALUES (?, ?, ?, ?, ?)
        `, [username, email || null, passwordHash, role_id || null, group_id || null]);

        const insertId = result?.lastInsertRowid || result?.insertId;
        logger.audit('user_created', req.admin.id, 'admin', insertId, { username });

        res.status(201).json({ id: insertId, message: 'User created' });
    } catch (error) {
        logger.error('Create user error', { error: error.message });
        res.status(500).json({ error: 'Failed to create user' });
    }
});

/**
 * PATCH /api/admin/users/:id
 * Update admin user
 */
router.patch('/users/:id', requirePermission('admin.users.manage'), async (req, res) => {
    try {
        const { id } = req.params;
        const { email, role_id, group_id, is_active, password } = req.body;

        const user = await db.queryOne('SELECT * FROM admins WHERE id = ?', [id]);
        if (!user) {
            return res.status(404).json({ error: 'User not found' });
        }

        const updates = [];
        const params = [];

        if (email !== undefined) { updates.push('email = ?'); params.push(email); }
        if (role_id !== undefined) { updates.push('role_id = ?'); params.push(role_id); }
        if (group_id !== undefined) { updates.push('group_id = ?'); params.push(group_id); }
        if (is_active !== undefined) { updates.push('is_active = ?'); params.push(is_active ? 1 : 0); }
        if (password) {
            const hash = await bcrypt.hash(password, 10);
            updates.push('password_hash = ?');
            params.push(hash);
        }

        if (updates.length === 0) {
            return res.status(400).json({ error: 'No valid fields to update' });
        }

        updates.push('updated_at = CURRENT_TIMESTAMP');
        params.push(id);

        await db.run(`UPDATE admins SET ${updates.join(', ')} WHERE id = ?`, params);
        clearPermissionCache(parseInt(id));
        logger.audit('user_updated', req.admin.id, 'admin', id, req.body);

        res.json({ message: 'User updated' });
    } catch (error) {
        logger.error('Update user error', { error: error.message });
        res.status(500).json({ error: 'Failed to update user' });
    }
});

/**
 * DELETE /api/admin/users/:id
 * Delete admin user (soft delete)
 */
router.delete('/users/:id', requirePermission('admin.users.manage'), async (req, res) => {
    try {
        const { id } = req.params;

        if (parseInt(id) === req.admin.id) {
            return res.status(400).json({ error: 'Cannot delete yourself' });
        }

        await db.run('UPDATE admins SET is_active = 0, updated_at = CURRENT_TIMESTAMP WHERE id = ?', [id]);
        clearPermissionCache(parseInt(id));
        logger.audit('user_deleted', req.admin.id, 'admin', id);

        res.json({ message: 'User deactivated' });
    } catch (error) {
        logger.error('Delete user error', { error: error.message });
        res.status(500).json({ error: 'Failed to delete user' });
    }
});

// ==================== ROLES ====================

/**
 * GET /api/admin/roles
 * List all roles with their permissions
 */
router.get('/roles', requirePermission('admin.roles.view'), async (req, res) => {
    try {
        const roles = await db.query('SELECT * FROM roles ORDER BY is_system DESC, name ASC');

        // Get permissions for each role
        for (const role of roles || []) {
            const perms = await db.query(`
                SELECT p.name FROM permissions p
                JOIN role_permissions rp ON p.id = rp.permission_id
                WHERE rp.role_id = ?
            `, [role.id]);
            role.permissions = (perms || []).map(p => p.name);
        }

        res.json(roles || []);
    } catch (error) {
        logger.error('List roles error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch roles' });
    }
});

/**
 * POST /api/admin/roles
 * Create new role
 */
router.post('/roles', requirePermission('admin.roles.manage'), async (req, res) => {
    try {
        const { name, description, permissions } = req.body;

        if (!name) {
            return res.status(400).json({ error: 'Role name required' });
        }

        const result = await db.run(
            'INSERT INTO roles (name, description) VALUES (?, ?)',
            [name, description || '']
        );
        const roleId = result?.lastInsertRowid || result?.insertId;

        // Assign permissions
        if (permissions && Array.isArray(permissions)) {
            for (const permName of permissions) {
                const perm = await db.queryOne('SELECT id FROM permissions WHERE name = ?', [permName]);
                if (perm) {
                    await db.run('INSERT INTO role_permissions (role_id, permission_id) VALUES (?, ?)', [roleId, perm.id]);
                }
            }
        }

        logger.audit('role_created', req.admin.id, 'role', roleId, { name });
        res.status(201).json({ id: roleId, message: 'Role created' });
    } catch (error) {
        logger.error('Create role error', { error: error.message });
        res.status(500).json({ error: 'Failed to create role' });
    }
});

/**
 * PUT /api/admin/roles/:id
 * Update role and its permissions
 */
router.put('/roles/:id', requirePermission('admin.roles.manage'), async (req, res) => {
    try {
        const { id } = req.params;
        const { name, description, permissions } = req.body;

        const role = await db.queryOne('SELECT * FROM roles WHERE id = ?', [id]);
        if (!role) {
            return res.status(404).json({ error: 'Role not found' });
        }
        if (role.is_system) {
            return res.status(403).json({ error: 'Cannot modify system roles' });
        }

        await db.run(
            'UPDATE roles SET name = ?, description = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?',
            [name || role.name, description !== undefined ? description : role.description, id]
        );

        // Update permissions
        if (permissions && Array.isArray(permissions)) {
            await db.run('DELETE FROM role_permissions WHERE role_id = ?', [id]);
            for (const permName of permissions) {
                const perm = await db.queryOne('SELECT id FROM permissions WHERE name = ?', [permName]);
                if (perm) {
                    await db.run('INSERT INTO role_permissions (role_id, permission_id) VALUES (?, ?)', [id, perm.id]);
                }
            }
        }

        logger.audit('role_updated', req.admin.id, 'role', id, { name, permissions });
        res.json({ message: 'Role updated' });
    } catch (error) {
        logger.error('Update role error', { error: error.message });
        res.status(500).json({ error: 'Failed to update role' });
    }
});

/**
 * DELETE /api/admin/roles/:id
 * Delete role
 */
router.delete('/roles/:id', requirePermission('admin.roles.manage'), async (req, res) => {
    try {
        const { id } = req.params;

        const role = await db.queryOne('SELECT * FROM roles WHERE id = ?', [id]);
        if (!role) {
            return res.status(404).json({ error: 'Role not found' });
        }
        if (role.is_system) {
            return res.status(403).json({ error: 'Cannot delete system roles' });
        }

        // Check if role is in use
        const usersWithRole = await db.queryOne('SELECT COUNT(*) as count FROM admins WHERE role_id = ?', [id]);
        if (usersWithRole?.count > 0) {
            return res.status(400).json({ error: 'Role is assigned to users' });
        }

        await db.run('DELETE FROM role_permissions WHERE role_id = ?', [id]);
        await db.run('DELETE FROM roles WHERE id = ?', [id]);

        logger.audit('role_deleted', req.admin.id, 'role', id);
        res.json({ message: 'Role deleted' });
    } catch (error) {
        logger.error('Delete role error', { error: error.message });
        res.status(500).json({ error: 'Failed to delete role' });
    }
});

// ==================== GROUPS ====================

/**
 * GET /api/admin/groups
 * List all groups
 */
router.get('/groups', requirePermission('admin.groups.view'), async (req, res) => {
    try {
        const groups = await db.query('SELECT * FROM `groups` ORDER BY name ASC');
        res.json(groups || []);
    } catch (error) {
        logger.error('List groups error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch groups' });
    }
});

/**
 * POST /api/admin/groups
 * Create new group
 */
router.post('/groups', requirePermission('admin.groups.manage'), async (req, res) => {
    try {
        const { name, description } = req.body;

        if (!name) {
            return res.status(400).json({ error: 'Group name required' });
        }

        const result = await db.run(
            'INSERT INTO `groups` (name, description) VALUES (?, ?)',
            [name, description || '']
        );
        const groupId = result?.lastInsertRowid || result?.insertId;

        logger.audit('group_created', req.admin.id, 'group', groupId, { name });
        res.status(201).json({ id: groupId, message: 'Group created' });
    } catch (error) {
        logger.error('Create group error', { error: error.message });
        res.status(500).json({ error: 'Failed to create group' });
    }
});

/**
 * DELETE /api/admin/groups/:id
 * Delete group
 */
router.delete('/groups/:id', requirePermission('admin.groups.manage'), async (req, res) => {
    try {
        const { id } = req.params;

        await db.run('UPDATE admins SET group_id = NULL WHERE group_id = ?', [id]);
        await db.run('DELETE FROM `groups` WHERE id = ?', [id]);

        logger.audit('group_deleted', req.admin.id, 'group', id);
        res.json({ message: 'Group deleted' });
    } catch (error) {
        logger.error('Delete group error', { error: error.message });
        res.status(500).json({ error: 'Failed to delete group' });
    }
});

/**
 * PUT /api/admin/groups/:id
 * Update group
 */
router.put('/groups/:id', requirePermission('admin.groups.manage'), async (req, res) => {
    try {
        const { id } = req.params;
        const { name, description } = req.body;

        const group = await db.queryOne('SELECT * FROM `groups` WHERE id = ?', [id]);
        if (!group) {
            return res.status(404).json({ error: 'Group not found' });
        }

        await db.run(
            'UPDATE `groups` SET name = ?, description = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?',
            [name || group.name, description !== undefined ? description : group.description, id]
        );

        logger.audit('group_updated', req.admin.id, 'group', id, { name, description });
        res.json({ message: 'Group updated' });
    } catch (error) {
        logger.error('Update group error', { error: error.message });
        res.status(500).json({ error: 'Failed to update group' });
    }
});

// ==================== PERMISSIONS ====================

/**
 * GET /api/admin/permissions
 * List all available permissions
 */
router.get('/permissions', requirePermission('admin.roles.view'), async (req, res) => {
    try {
        res.json(PERMISSIONS);
    } catch (error) {
        logger.error('List permissions error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch permissions' });
    }
});

module.exports = router;

