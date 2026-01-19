/**
 * Permissions Hook
 * Provides permission checking utilities for components
 */

import { createContext, useContext, useMemo } from 'react';

// Create context for permissions
export const PermissionsContext = createContext({
  permissions: [],
  hasPermission: () => false,
  hasAnyPermission: () => false,
  hasAllPermissions: () => false,
});

/**
 * Hook to access permissions
 */
export function usePermissions() {
  return useContext(PermissionsContext);
}

/**
 * Create permissions value for context provider
 */
export function createPermissionsValue(permissions = []) {
  return {
    permissions,
    
    /**
     * Check if user has a specific permission
     */
    hasPermission: (permission) => {
      return permissions.includes(permission);
    },
    
    /**
     * Check if user has any of the specified permissions
     */
    hasAnyPermission: (...requiredPermissions) => {
      return requiredPermissions.some(p => permissions.includes(p));
    },
    
    /**
     * Check if user has all of the specified permissions
     */
    hasAllPermissions: (...requiredPermissions) => {
      return requiredPermissions.every(p => permissions.includes(p));
    },
  };
}

/**
 * Permission-based component wrapper
 * Only renders children if user has required permission
 */
export function RequirePermission({ permission, permissions: requiredPermissions, any = false, children, fallback = null }) {
  const { hasPermission, hasAnyPermission, hasAllPermissions } = usePermissions();
  
  let hasAccess = false;
  
  if (permission) {
    hasAccess = hasPermission(permission);
  } else if (requiredPermissions) {
    hasAccess = any 
      ? hasAnyPermission(...requiredPermissions)
      : hasAllPermissions(...requiredPermissions);
  }
  
  return hasAccess ? children : fallback;
}

/**
 * Navigation items with required permissions
 */
export const NAV_ITEMS = [
  { path: '/', label: 'Dashboard', icon: '📊', permission: 'dashboard.view' },
  { path: '/players', label: 'Players', icon: '👥', permission: 'players.view' },
  { path: '/detections', label: 'Detections', icon: '🔍', permission: 'detections.view' },
  { path: '/sanctions', label: 'Sanctions', icon: '🔨', permission: 'sanctions.view' },
  { path: '/rulesets', label: 'Rulesets', icon: '⚙️', permission: 'rulesets.view' },
  { path: '/live', label: 'Live Monitor', icon: '📡', permission: 'live.view' },
  { path: '/admin', label: 'Admin', icon: '👤', permission: 'admin.users.view' },
];

