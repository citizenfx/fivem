/**
 * Rate Limiting Middleware
 * Protects against brute force attacks and DoS
 */

const logger = require('../services/logger');

// In-memory store for rate limiting (consider Redis for multi-instance deployments)
const requestCounts = new Map();
const blockedIps = new Map();

// Configuration
const config = {
    // General API rate limit
    api: {
        windowMs: 60 * 1000,  // 1 minute
        maxRequests: 100,     // 100 requests per minute
    },
    // Login rate limit (stricter)
    login: {
        windowMs: 15 * 60 * 1000, // 15 minutes
        maxRequests: 5,           // 5 attempts per 15 minutes
        blockDuration: 30 * 60 * 1000, // Block for 30 minutes after exceeding
    },
    // FiveM API rate limit
    fivem: {
        windowMs: 1000,       // 1 second
        maxRequests: 50,      // 50 requests per second
    }
};

/**
 * Clean up expired entries periodically
 */
setInterval(() => {
    const now = Date.now();
    
    // Clean request counts
    for (const [key, data] of requestCounts.entries()) {
        if (now - data.windowStart > data.windowMs * 2) {
            requestCounts.delete(key);
        }
    }
    
    // Clean blocked IPs
    for (const [ip, expiry] of blockedIps.entries()) {
        if (now > expiry) {
            blockedIps.delete(ip);
            logger.info('Rate limit block expired', { ip });
        }
    }
}, 60 * 1000); // Run every minute

/**
 * Create a rate limiter for a specific type
 */
function createRateLimiter(type) {
    const cfg = config[type] || config.api;
    
    return (req, res, next) => {
        const ip = req.ip || req.connection.remoteAddress;
        const key = `${type}:${ip}`;
        const now = Date.now();
        
        // Check if IP is blocked
        if (blockedIps.has(ip)) {
            const expiry = blockedIps.get(ip);
            if (now < expiry) {
                const remainingMs = expiry - now;
                logger.warn('Blocked IP attempted request', { ip, type, remainingMs });
                return res.status(429).json({
                    error: 'Too many requests. Please try again later.',
                    retryAfter: Math.ceil(remainingMs / 1000)
                });
            }
            blockedIps.delete(ip);
        }
        
        // Get or create request count entry
        let entry = requestCounts.get(key);
        
        if (!entry || now - entry.windowStart > cfg.windowMs) {
            entry = {
                count: 0,
                windowStart: now,
                windowMs: cfg.windowMs
            };
            requestCounts.set(key, entry);
        }
        
        entry.count++;
        
        // Check if limit exceeded
        if (entry.count > cfg.maxRequests) {
            logger.warn('Rate limit exceeded', { ip, type, count: entry.count });
            
            // Block IP if this is a login attempt
            if (type === 'login' && cfg.blockDuration) {
                blockedIps.set(ip, now + cfg.blockDuration);
                logger.warn('IP blocked due to repeated login failures', { 
                    ip, 
                    blockDuration: cfg.blockDuration / 1000 
                });
            }
            
            return res.status(429).json({
                error: 'Too many requests. Please try again later.',
                retryAfter: Math.ceil((entry.windowStart + cfg.windowMs - now) / 1000)
            });
        }
        
        // Add rate limit headers
        res.set({
            'X-RateLimit-Limit': cfg.maxRequests,
            'X-RateLimit-Remaining': Math.max(0, cfg.maxRequests - entry.count),
            'X-RateLimit-Reset': Math.ceil((entry.windowStart + cfg.windowMs) / 1000)
        });
        
        next();
    };
}

// Export pre-configured rate limiters
module.exports = {
    apiLimiter: createRateLimiter('api'),
    loginLimiter: createRateLimiter('login'),
    fivemLimiter: createRateLimiter('fivem'),
    createRateLimiter
};

