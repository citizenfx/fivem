/**
 * Request logging middleware
 */

const logger = require('../services/logger');

function requestLogger(req, res, next) {
    const start = Date.now();

    // Log on response finish
    res.on('finish', () => {
        const duration = Date.now() - start;
        
        // Skip health checks and static assets
        if (req.path === '/api/fivem/health' || req.path.startsWith('/static')) {
            return;
        }

        const logData = {
            method: req.method,
            path: req.path,
            status: res.statusCode,
            duration_ms: duration,
            ip: req.ip,
            user_agent: req.headers['user-agent']
        };

        if (req.admin) {
            logData.admin = req.admin.username;
        }
        if (req.fivemServer) {
            logData.fivem_server = req.fivemServer.name;
        }

        // Log based on status code
        if (res.statusCode >= 500) {
            logger.error('request', logData);
        } else if (res.statusCode >= 400) {
            logger.warn('request', logData);
        } else {
            logger.api('request', logData);
        }
    });

    next();
}

module.exports = { requestLogger };
