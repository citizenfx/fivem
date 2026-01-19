/**
 * Permissions Service
 * Defines all permissions and provides permission checking utilities
 */

// Permission definitions organized by category
const PERMISSIONS = {
    // Dashboard
    'dashboard.view': { category: 'dashboard', description: 'View dashboard statistics' },
    
    // Players
    'players.view': { category: 'players', description: 'View player list and details' },
    'players.edit': { category: 'players', description: 'Edit player notes and trust level' },
    
    // Detections
    'detections.view': { category: 'detections', description: 'View detection logs' },
    'detections.delete': { category: 'detections', description: 'Delete detection records' },
    
    // Sanctions
    'sanctions.view': { category: 'sanctions', description: 'View sanctions list' },
    'sanctions.create': { category: 'sanctions', description: 'Create new sanctions (warn/kick/ban)' },
    'sanctions.edit': { category: 'sanctions', description: 'Edit existing sanctions' },
    'sanctions.revoke': { category: 'sanctions', description: 'Revoke active sanctions' },
    
    // Rulesets
    'rulesets.view': { category: 'rulesets', description: 'View ruleset configurations' },
    'rulesets.create': { category: 'rulesets', description: 'Create new rulesets' },
    'rulesets.edit': { category: 'rulesets', description: 'Edit ruleset configurations' },
    'rulesets.delete': { category: 'rulesets', description: 'Delete rulesets' },
    'rulesets.activate': { category: 'rulesets', description: 'Activate/deactivate rulesets' },
    
    // Live Monitor
    'live.view': { category: 'live', description: 'View live monitor' },
    
    // Admin Management
    'admin.users.view': { category: 'admin', description: 'View admin users list' },
    'admin.users.create': { category: 'admin', description: 'Create new admin users' },
    'admin.users.edit': { category: 'admin', description: 'Edit admin users' },
    'admin.users.delete': { category: 'admin', description: 'Delete admin users' },
    'admin.roles.view': { category: 'admin', description: 'View roles and permissions' },
    'admin.roles.manage': { category: 'admin', description: 'Create/edit/delete roles' },
    'admin.groups.view': { category: 'admin', description: 'View admin groups' },
    'admin.groups.manage': { category: 'admin', description: 'Create/edit/delete groups' },
    'admin.audit.view': { category: 'admin', description: 'View audit log' },
};

// Default role definitions with their permissions
const DEFAULT_ROLES = {
    superadmin: {
        description: 'Full system access with all permissions',
        isSystem: true,
        permissions: Object.keys(PERMISSIONS) // All permissions
    },
    admin: {
        description: 'Administrative access without user management',
        isSystem: true,
        permissions: [
            'dashboard.view',
            'players.view', 'players.edit',
            'detections.view', 'detections.delete',
            'sanctions.view', 'sanctions.create', 'sanctions.edit', 'sanctions.revoke',
            'rulesets.view', 'rulesets.create', 'rulesets.edit', 'rulesets.delete', 'rulesets.activate',
            'live.view',
            'admin.users.view', 'admin.roles.view', 'admin.groups.view', 'admin.audit.view'
        ]
    },
    moderator: {
        description: 'Basic moderation access',
        isSystem: true,
        permissions: [
            'dashboard.view',
            'players.view',
            'detections.view',
            'sanctions.view', 'sanctions.create',
            'rulesets.view',
            'live.view'
        ]
    },
    viewer: {
        description: 'Read-only access to view data',
        isSystem: true,
        permissions: [
            'dashboard.view',
            'players.view',
            'detections.view',
            'sanctions.view',
            'rulesets.view',
            'live.view'
        ]
    }
};

/**
 * Get all permission definitions
 */
function getAllPermissions() {
    return PERMISSIONS;
}

/**
 * Get permissions grouped by category
 */
function getPermissionsByCategory() {
    const categories = {};
    for (const [name, info] of Object.entries(PERMISSIONS)) {
        if (!categories[info.category]) {
            categories[info.category] = [];
        }
        categories[info.category].push({ name, ...info });
    }
    return categories;
}

/**
 * Get default role definitions
 */
function getDefaultRoles() {
    return DEFAULT_ROLES;
}

/**
 * Check if a permission exists
 */
function isValidPermission(permission) {
    return permission in PERMISSIONS;
}

module.exports = {
    PERMISSIONS,
    DEFAULT_ROLES,
    getAllPermissions,
    getPermissionsByCategory,
    getDefaultRoles,
    isValidPermission
};

