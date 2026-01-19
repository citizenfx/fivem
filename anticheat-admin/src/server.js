/**
 * Anticheat Admin Server
 * Main Express application entry point
 */

const express = require('express');
const cors = require('cors');
const helmet = require('helmet');
const http = require('http');
const WebSocket = require('ws');
const path = require('path');

const logger = require('./services/logger');
const { requestLogger } = require('./middleware/logger');
const { apiLimiter, loginLimiter, fivemLimiter } = require('./middleware/rateLimit');
const { initDb, getDbType, closeDb } = require('./db');

// Route imports
const authRoutes = require('./routes/auth');
const playersRoutes = require('./routes/players');
const detectionsRoutes = require('./routes/detections');
const sanctionsRoutes = require('./routes/sanctions');
const rulesetsRoutes = require('./routes/rulesets');
const statsRoutes = require('./routes/stats');
const adminRoutes = require('./routes/admin');
const { router: fivemRoutes, setWsClients } = require('./routes/fivem');

const app = express();
const PORT = process.env.PORT || 3000;

// ============================================================================
// Middleware
// ============================================================================

// Security middleware
app.use(helmet({
    contentSecurityPolicy: false // Disable for development
}));

// CORS configuration
app.use(cors({
    origin: process.env.CORS_ORIGIN || '*',
    methods: ['GET', 'POST', 'PUT', 'PATCH', 'DELETE'],
    allowedHeaders: ['Content-Type', 'Authorization', 'X-API-Key']
}));

// Body parsing
app.use(express.json({ limit: '10mb' }));
app.use(express.urlencoded({ extended: true }));

// Request logging
app.use(requestLogger);

// ============================================================================
// API Routes (with rate limiting)
// ============================================================================

// Auth routes with stricter rate limiting on login
app.use('/api/auth/login', loginLimiter);
app.use('/api/auth', authRoutes);

// Standard API routes with general rate limiting
app.use('/api/players', apiLimiter, playersRoutes);
app.use('/api/detections', apiLimiter, detectionsRoutes);
app.use('/api/sanctions', apiLimiter, sanctionsRoutes);
app.use('/api/rulesets', apiLimiter, rulesetsRoutes);
app.use('/api/stats', apiLimiter, statsRoutes);
app.use('/api/admin', apiLimiter, adminRoutes);

// FiveM routes with higher rate limit
app.use('/api/fivem', fivemLimiter, fivemRoutes);

// Health check
app.get('/health', (req, res) => {
    res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

// Serve static frontend in production
if (process.env.NODE_ENV === 'production') {
    app.use(express.static(path.join(__dirname, '../frontend/dist')));
    
    app.get('*', (req, res) => {
        res.sendFile(path.join(__dirname, '../frontend/dist/index.html'));
    });
}

// ============================================================================
// Error Handling
// ============================================================================

// 404 handler
app.use((req, res) => {
    res.status(404).json({ error: 'Endpoint not found' });
});

// Global error handler
app.use((err, req, res, next) => {
    logger.error('Unhandled error', { 
        error: err.message, 
        stack: err.stack,
        path: req.path 
    });
    res.status(500).json({ error: 'Internal server error' });
});

// ============================================================================
// Server & WebSocket Setup
// ============================================================================

const server = http.createServer(app);

// WebSocket server for real-time updates
const wss = new WebSocket.Server({ server, path: '/ws' });
const wsClients = [];

wss.on('connection', (ws, req) => {
    logger.info('WebSocket client connected', { ip: req.socket.remoteAddress });
    wsClients.push(ws);

    ws.on('close', () => {
        const index = wsClients.indexOf(ws);
        if (index > -1) {
            wsClients.splice(index, 1);
        }
        logger.info('WebSocket client disconnected');
    });

    ws.on('error', (error) => {
        logger.error('WebSocket error', { error: error.message });
    });

    // Send initial connection confirmation
    ws.send(JSON.stringify({
        event: 'connected',
        data: { message: 'Connected to anticheat admin WebSocket' },
        timestamp: new Date().toISOString()
    }));
});

// Pass WebSocket clients to fivem routes for broadcasting
setWsClients(wsClients);

// ============================================================================
// Start Server (with async database initialization)
// ============================================================================

async function startServer() {
    try {
        // Initialize database FIRST before starting server
        console.log('Initializing database...');
        await initDb();
        const dbType = getDbType();
        console.log(`Database ready! (${dbType})`);

        server.listen(PORT, () => {
            logger.info('Server started', {
                port: PORT,
                env: process.env.NODE_ENV || 'development',
                database: dbType
            });
            console.log(`
╔════════════════════════════════════════════════════════════╗
║                                                            ║
║   🛡️  Anticheat Admin Server                              ║
║                                                            ║
║   Server running on: http://localhost:${PORT}                ║
║   WebSocket: ws://localhost:${PORT}/ws                       ║
║   Database: ${dbType.padEnd(44)}║
║                                                            ║
║   API Endpoints:                                           ║
║     - /api/auth      (Authentication)                      ║
║     - /api/players   (Player Management)                   ║
║     - /api/detections (Detection Logs)                     ║
║     - /api/sanctions (Ban/Kick Management)                 ║
║     - /api/rulesets  (Ruleset Configuration)               ║
║     - /api/stats     (Dashboard Statistics)                ║
║     - /api/fivem     (FiveM Server Integration)            ║
║                                                            ║
╚════════════════════════════════════════════════════════════╝
            `);
        });
    } catch (error) {
        console.error('Failed to start server:', error);
        process.exit(1);
    }
}

// Start the server
startServer();

// Graceful shutdown
async function gracefulShutdown(signal) {
    logger.info(`${signal} received, shutting down gracefully`);

    // Close database connection
    try {
        await closeDb();
        logger.info('Database connection closed');
    } catch (err) {
        logger.error('Error closing database', { error: err.message });
    }

    server.close(() => {
        logger.info('Server closed');
        process.exit(0);
    });
}

process.on('SIGTERM', () => gracefulShutdown('SIGTERM'));
process.on('SIGINT', () => gracefulShutdown('SIGINT'));

module.exports = { app, server };
