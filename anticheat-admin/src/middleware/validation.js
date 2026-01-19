/**
 * Input Validation Middleware
 * Validates and sanitizes request inputs
 */

const logger = require('../services/logger');

/**
 * Validation rules
 */
const patterns = {
    license: /^license:[a-f0-9]{40}$/i,
    steamId: /^steam:[0-9a-f]+$/i,
    discordId: /^\d{17,19}$/,
    username: /^[a-zA-Z0-9_-]{3,50}$/,
    email: /^[^\s@]+@[^\s@]+\.[^\s@]+$/,
    sanctionType: /^(warn|kick|ban)$/,
    severity: /^(low|medium|high|critical)$/,
    role: /^(moderator|admin|superadmin)$/,
    uuid: /^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/i,
};

/**
 * Sanitize string - remove dangerous characters
 */
function sanitizeString(str, maxLength = 1000) {
    if (typeof str !== 'string') return str;
    return str
        .slice(0, maxLength)
        .replace(/[<>]/g, '') // Remove HTML-like tags
        .trim();
}

/**
 * Sanitize object recursively
 */
function sanitizeObject(obj, depth = 0) {
    if (depth > 10) return obj; // Prevent deep recursion
    
    if (typeof obj === 'string') {
        return sanitizeString(obj);
    }
    
    if (Array.isArray(obj)) {
        return obj.map(item => sanitizeObject(item, depth + 1));
    }
    
    if (obj && typeof obj === 'object') {
        const sanitized = {};
        for (const [key, value] of Object.entries(obj)) {
            sanitized[sanitizeString(key, 100)] = sanitizeObject(value, depth + 1);
        }
        return sanitized;
    }
    
    return obj;
}

/**
 * Create a validation middleware
 * @param {Object} schema - Validation schema
 */
function validate(schema) {
    return (req, res, next) => {
        const errors = [];
        
        // Validate body
        if (schema.body) {
            for (const [field, rules] of Object.entries(schema.body)) {
                const value = req.body[field];
                const error = validateField(field, value, rules);
                if (error) errors.push(error);
            }
        }
        
        // Validate query params
        if (schema.query) {
            for (const [field, rules] of Object.entries(schema.query)) {
                const value = req.query[field];
                const error = validateField(field, value, rules);
                if (error) errors.push(error);
            }
        }
        
        // Validate path params
        if (schema.params) {
            for (const [field, rules] of Object.entries(schema.params)) {
                const value = req.params[field];
                const error = validateField(field, value, rules);
                if (error) errors.push(error);
            }
        }
        
        if (errors.length > 0) {
            logger.warn('Validation failed', { errors, ip: req.ip });
            return res.status(400).json({ 
                error: 'Validation failed',
                details: errors 
            });
        }
        
        // Sanitize body
        if (req.body) {
            req.body = sanitizeObject(req.body);
        }
        
        next();
    };
}

/**
 * Validate a single field
 */
function validateField(field, value, rules) {
    // Check required
    if (rules.required && (value === undefined || value === null || value === '')) {
        return `${field} is required`;
    }
    
    // Skip further validation if not required and empty
    if (value === undefined || value === null || value === '') {
        return null;
    }
    
    // Check type
    if (rules.type) {
        const actualType = Array.isArray(value) ? 'array' : typeof value;
        if (actualType !== rules.type) {
            return `${field} must be a ${rules.type}`;
        }
    }
    
    // Check pattern
    if (rules.pattern && typeof value === 'string') {
        const pattern = typeof rules.pattern === 'string' ? patterns[rules.pattern] : rules.pattern;
        if (pattern && !pattern.test(value)) {
            return `${field} has invalid format`;
        }
    }
    
    // Check min/max length for strings
    if (typeof value === 'string') {
        if (rules.minLength && value.length < rules.minLength) {
            return `${field} must be at least ${rules.minLength} characters`;
        }
        if (rules.maxLength && value.length > rules.maxLength) {
            return `${field} must be at most ${rules.maxLength} characters`;
        }
    }
    
    // Check min/max for numbers
    if (typeof value === 'number') {
        if (rules.min !== undefined && value < rules.min) {
            return `${field} must be at least ${rules.min}`;
        }
        if (rules.max !== undefined && value > rules.max) {
            return `${field} must be at most ${rules.max}`;
        }
    }
    
    // Check enum values
    if (rules.enum && !rules.enum.includes(value)) {
        return `${field} must be one of: ${rules.enum.join(', ')}`;
    }
    
    return null;
}

module.exports = {
    validate,
    sanitizeString,
    sanitizeObject,
    patterns
};

