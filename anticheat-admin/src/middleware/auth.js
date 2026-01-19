/**
 * JWT Authentication Middleware with RBAC
 */

const jwt = require('jsonwebtoken');
const logger = require('../services/logger');
const db = require('../db');

const JWT_SECRET = process.env.JWT_SECRET || 'change-this-secret-in-production';
const JWT_EXPIRY = process.env.JWT_EXPIRY || '24h';

// Cache for user permissions (cleared on role changes)
const permissionCache = new Map();
const CACHE_TTL = 5 * 60 * 1000; // 5 minutes

/**
 * Generate JWT token for admin user
 */
function generateToken(admin) {
    return jwt.sign(
        {
            id: admin.id,
            username: admin.username,
            role_id: admin.role_id,
            role_name: admin.role_name
        },
        JWT_SECRET,
        { expiresIn: JWT_EXPIRY }
    );
}

/**
 * Verify JWT token middleware
 */
function authenticateToken(req, res, next) {
    const authHeader = req.headers['authorization'];
    const token = authHeader && authHeader.split(' ')[1]; // Bearer TOKEN

    if (!token) {
        logger.auth('auth_failed', { reason: 'no_token', ip: req.ip });
        return res.status(401).json({ error: 'Authentication required' });
    }

    jwt.verify(token, JWT_SECRET, (err, decoded) => {
        if (err) {
            logger.auth('auth_failed', { reason: 'invalid_token', ip: req.ip });
            return res.status(403).json({ error: 'Invalid or expired token' });
        }

        req.admin = decoded;
        next();
    });
}

/**
 * Get user permissions from database (with caching)
 */
async function getUserPermissions(userId) {
    const cacheKey = `user_${userId}`;
    const cached = permissionCache.get(cacheKey);

    if (cached && Date.now() - cached.timestamp < CACHE_TTL) {
        return cached.permissions;
    }

    // Fetch permissions from role_permissions join
    const permissions = await db.query(`
        SELECT p.name
        FROM permissions p
        INNER JOIN role_permissions rp ON rp.permission_id = p.id
        INNER JOIN admins a ON a.role_id = rp.role_id
        WHERE a.id = ?
    `, [userId]);

    const permissionNames = (permissions || []).map(p => p.name);

    permissionCache.set(cacheKey, {
        permissions: permissionNames,
        timestamp: Date.now()
    });

    return permissionNames;
}

/**
 * Clear permission cache for a user
 */
function clearPermissionCache(userId) {
    if (userId) {
        permissionCache.delete(`user_${userId}`);
    } else {
        permissionCache.clear();
    }
}

/**
 * Permission-based authorization middleware
 * @param {...string} requiredPermissions - One or more permissions (user needs at least one)
 */
function requirePermission(...requiredPermissions) {
    return async (req, res, next) => {
        if (!req.admin) {
            return res.status(401).json({ error: 'Authentication required' });
        }

        try {
            const userPermissions = await getUserPermissions(req.admin.id);
            req.admin.permissions = userPermissions;

            // Check if user has ANY of the required permissions
            const hasPermission = requiredPermissions.some(p => userPermissions.includes(p));

            if (!hasPermission) {
                logger.auth('auth_forbidden', {
                    admin: req.admin.username,
                    required: requiredPermissions,
                    has: userPermissions
                });
                return res.status(403).json({ error: 'Insufficient permissions' });
            }

            next();
        } catch (error) {
            logger.error('Permission check error', { error: error.message });
            return res.status(500).json({ error: 'Permission check failed' });
        }
    };
}

/**
 * Role-based authorization middleware (legacy support)
 */
function requireRole(...allowedRoles) {
    return (req, res, next) => {
        if (!req.admin) {
            return res.status(401).json({ error: 'Authentication required' });
        }

        if (!allowedRoles.includes(req.admin.role_name)) {
            logger.auth('auth_forbidden', {
                admin: req.admin.username,
                required: allowedRoles,
                has: req.admin.role_name
            });
            return res.status(403).json({ error: 'Insufficient permissions' });
        }

        next();
    };
}

module.exports = {
    generateToken,
    authenticateToken,
    requirePermission,
    requireRole,
    getUserPermissions,
    clearPermissionCache,
    JWT_SECRET
};
