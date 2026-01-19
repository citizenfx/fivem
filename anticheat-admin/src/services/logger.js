/**
 * Winston Logger Service
 * Configured for ELK Stack integration with JSON output
 */

const winston = require('winston');
const path = require('path');
const fs = require('fs');

// Ensure log directory exists
const LOG_DIR = process.env.LOG_DIR || path.join(__dirname, '../../logs');
if (!fs.existsSync(LOG_DIR)) {
    fs.mkdirSync(LOG_DIR, { recursive: true });
}

const logger = winston.createLogger({
    level: process.env.LOG_LEVEL || 'info',
    format: winston.format.combine(
        winston.format.timestamp({ format: 'YYYY-MM-DDTHH:mm:ss.SSSZ' }),
        winston.format.errors({ stack: true }),
        winston.format.json()
    ),
    defaultMeta: { 
        service: 'anticheat-admin',
        application: 'fivem-anticheat'
    },
    transports: [
        // JSON file transport for ELK Stack
        new winston.transports.File({
            filename: path.join(LOG_DIR, 'admin.log'),
            maxsize: 10485760, // 10MB
            maxFiles: 5,
            tailable: true
        }),
        // Error-only file
        new winston.transports.File({
            filename: path.join(LOG_DIR, 'error.log'),
            level: 'error',
            maxsize: 10485760,
            maxFiles: 5
        })
    ]
});

// Console transport for development
if (process.env.NODE_ENV !== 'production') {
    logger.add(new winston.transports.Console({
        format: winston.format.combine(
            winston.format.colorize(),
            winston.format.timestamp({ format: 'HH:mm:ss' }),
            winston.format.printf(({ level, message, timestamp, ...meta }) => {
                const metaStr = Object.keys(meta).length > 2 
                    ? ` ${JSON.stringify(meta)}` 
                    : '';
                return `${timestamp} ${level}: ${message}${metaStr}`;
            })
        )
    }));
}

// Helper methods for structured logging
logger.api = (action, details = {}) => {
    logger.info(action, { module: 'api', ...details });
};

logger.auth = (action, details = {}) => {
    logger.info(action, { module: 'auth', ...details });
};

logger.detection = (action, details = {}) => {
    logger.warn(action, { module: 'detection', ...details });
};

logger.sanction = (action, details = {}) => {
    logger.info(action, { module: 'sanction', ...details });
};

logger.audit = (action, adminId, entityType, entityId, details = {}) => {
    logger.info(action, {
        module: 'audit',
        admin_id: adminId,
        entity_type: entityType,
        entity_id: entityId,
        ...details
    });
};

module.exports = logger;
