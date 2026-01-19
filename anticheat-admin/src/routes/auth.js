/**
 * Authentication Routes
 */

const express = require('express');
const bcrypt = require('bcrypt');
const db = require('../db');
const { generateToken, authenticateToken, getUserPermissions } = require('../middleware/auth');
const logger = require('../services/logger');

const router = express.Router();

/**
 * POST /api/auth/login
 * Authenticate admin user
 */
router.post('/login', async (req, res) => {
    try {
        const { username, password } = req.body;

        if (!username || !password) {
            return res.status(400).json({ error: 'Username and password required' });
        }

        // Join with roles table to get role name
        const admin = await db.queryOne(`
            SELECT a.*, r.name as role_name
            FROM admins a
            LEFT JOIN roles r ON a.role_id = r.id
            WHERE a.username = ? AND a.is_active = 1
        `, [username]);

        if (!admin) {
            logger.auth('login_failed', { username, reason: 'user_not_found', ip: req.ip });
            return res.status(401).json({ error: 'Invalid credentials' });
        }

        const validPassword = await bcrypt.compare(password, admin.password_hash);
        if (!validPassword) {
            logger.auth('login_failed', { username, reason: 'invalid_password', ip: req.ip });
            return res.status(401).json({ error: 'Invalid credentials' });
        }

        // Update last login
        await db.run('UPDATE admins SET last_login = CURRENT_TIMESTAMP WHERE id = ?', [admin.id]);

        const token = generateToken(admin);

        // Get user permissions
        const permissions = await getUserPermissions(admin.id);

        logger.auth('login_success', { username, ip: req.ip });

        res.json({
            token,
            user: {
                id: admin.id,
                username: admin.username,
                role_id: admin.role_id,
                role: admin.role_name,
                email: admin.email,
                permissions
            }
        });
    } catch (error) {
        logger.error('Login error', { error: error.message });
        res.status(500).json({ error: 'Login failed' });
    }
});

/**
 * GET /api/auth/me
 * Get current user info with permissions
 */
router.get('/me', authenticateToken, async (req, res) => {
    try {
        const admin = await db.queryOne(`
            SELECT a.id, a.username, a.email, a.role_id, a.last_login, a.created_at,
                   r.name as role_name, g.name as group_name
            FROM admins a
            LEFT JOIN roles r ON a.role_id = r.id
            LEFT JOIN groups g ON a.group_id = g.id
            WHERE a.id = ?
        `, [req.admin.id]);

        if (!admin) {
            return res.status(404).json({ error: 'User not found' });
        }

        // Get user permissions
        const permissions = await getUserPermissions(admin.id);

        res.json({
            ...admin,
            role: admin.role_name,
            group: admin.group_name,
            permissions
        });
    } catch (error) {
        logger.error('Get user error', { error: error.message });
        res.status(500).json({ error: 'Failed to get user info' });
    }
});

/**
 * POST /api/auth/change-password
 * Change password
 */
router.post('/change-password', authenticateToken, async (req, res) => {
    try {
        const { currentPassword, newPassword } = req.body;

        if (!currentPassword || !newPassword) {
            return res.status(400).json({ error: 'Current and new password required' });
        }

        if (newPassword.length < 8) {
            return res.status(400).json({ error: 'Password must be at least 8 characters' });
        }

        const admin = await db.queryOne('SELECT * FROM admins WHERE id = ?', [req.admin.id]);
        const validPassword = await bcrypt.compare(currentPassword, admin.password_hash);

        if (!validPassword) {
            return res.status(401).json({ error: 'Current password is incorrect' });
        }

        const newHash = await bcrypt.hash(newPassword, 10);
        await db.run('UPDATE admins SET password_hash = ?, updated_at = CURRENT_TIMESTAMP WHERE id = ?',
            [newHash, req.admin.id]);

        logger.auth('password_changed', { admin: req.admin.username });

        res.json({ message: 'Password changed successfully' });
    } catch (error) {
        logger.error('Password change error', { error: error.message });
        res.status(500).json({ error: 'Failed to change password' });
    }
});

module.exports = router;
