/**
 * FiveM Server API Key Authentication Middleware
 */

const bcrypt = require('bcrypt');
const db = require('../db');
const logger = require('../services/logger');

/**
 * Validate FiveM server API key
 */
async function authenticateFiveM(req, res, next) {
    const apiKey = req.headers['x-api-key'];

    if (!apiKey) {
        logger.auth('fivem_auth_failed', { reason: 'no_api_key', ip: req.ip });
        return res.status(401).json({ error: 'API key required' });
    }

    try {
        // Get all active API keys and check against provided key
        const keys = db.query('SELECT * FROM api_keys WHERE is_active = 1');
        
        let validKey = null;
        for (const key of keys) {
            if (bcrypt.compareSync(apiKey, key.key_hash)) {
                validKey = key;
                break;
            }
        }

        if (!validKey) {
            logger.auth('fivem_auth_failed', { reason: 'invalid_key', ip: req.ip });
            return res.status(403).json({ error: 'Invalid API key' });
        }

        // Update last used timestamp
        db.run('UPDATE api_keys SET last_used = CURRENT_TIMESTAMP WHERE id = ?', [validKey.id]);

        req.fivemServer = {
            id: validKey.id,
            name: validKey.name,
            serverName: validKey.server_name
        };

        next();
    } catch (error) {
        logger.error('FiveM auth error', { error: error.message });
        res.status(500).json({ error: 'Authentication error' });
    }
}

module.exports = { authenticateFiveM };
