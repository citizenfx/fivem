/**
 * FiveM Server Integration Routes
 * Endpoints called by the FiveM anticheat resource
 */

const express = require('express');
const db = require('../db');
const { authenticateFiveM } = require('../middleware/fivemAuth');
const logger = require('../services/logger');

const router = express.Router();

// WebSocket clients for real-time updates
let wsClients = [];

function setWsClients(clients) {
    wsClients = clients;
}

function broadcast(event, data) {
    const message = JSON.stringify({ event, data, timestamp: new Date().toISOString() });
    wsClients.forEach(ws => {
        if (ws.readyState === 1) { // OPEN
            ws.send(message);
        }
    });
}

/**
 * GET /api/fivem/health
 * Health check endpoint
 */
router.get('/health', (req, res) => {
    res.json({ status: 'ok', timestamp: new Date().toISOString() });
});

/**
 * GET /api/fivem/rulesets/active
 * Fetch active ruleset for FiveM server
 */
router.get('/rulesets/active', authenticateFiveM, async (req, res) => {
    try {
        const ruleset = await db.queryOne('SELECT * FROM rulesets WHERE is_active = 1');

        if (!ruleset) {
            return res.status(404).json({ error: 'No active ruleset' });
        }

        const configJson = ruleset.config_json;
        const config = typeof configJson === 'string' ? JSON.parse(configJson) : configJson;

        res.json({
            id: ruleset.id,
            name: ruleset.name,
            version: ruleset.version,
            ...config
        });

        logger.api('ruleset_fetched', {
            server: req.fivemServer.name,
            ruleset: ruleset.name
        });
    } catch (error) {
        logger.error('FiveM ruleset fetch error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch ruleset' });
    }
});

/**
 * POST /api/fivem/detections
 * Report detection from FiveM server
 */
router.post('/detections', authenticateFiveM, async (req, res) => {
    try {
        const {
            player_license,
            player_name,
            detection_type,
            severity,
            data,
            timestamp
        } = req.body;

        if (!player_license || !detection_type) {
            return res.status(400).json({ error: 'Missing required fields' });
        }

        // Ensure player exists
        const player = await db.queryOne('SELECT id FROM players WHERE license = ?', [player_license]);
        let playerId = player?.id;

        if (!player) {
            const result = await db.run(`
                INSERT INTO players (license, name) VALUES (?, ?)
            `, [player_license, player_name || 'Unknown']);
            playerId = result?.lastInsertRowid || result?.insertId;
        } else {
            // Update player last seen and detection count
            await db.run(`
                UPDATE players
                SET last_seen = CURRENT_TIMESTAMP,
                    total_detections = total_detections + 1,
                    name = COALESCE(?, name)
                WHERE license = ?
            `, [player_name, player_license]);
        }

        // Insert detection
        const result = await db.run(`
            INSERT INTO detections (player_id, player_license, player_name, detection_type, severity, data, timestamp)
            VALUES (?, ?, ?, ?, ?, ?, ?)
        `, [
            playerId,
            player_license,
            player_name || 'Unknown',
            detection_type,
            severity || 'medium',
            data ? JSON.stringify(data) : null,
            timestamp || new Date().toISOString()
        ]);

        const insertId = result?.lastInsertRowid || result?.insertId;

        // Broadcast to WebSocket clients
        broadcast('detection', {
            id: insertId,
            player_license,
            player_name,
            detection_type,
            severity,
            data,
            timestamp
        });

        logger.detection('detection_received', {
            server: req.fivemServer.name,
            player: player_license,
            type: detection_type,
            severity
        });

        res.status(201).json({ id: insertId });
    } catch (error) {
        logger.error('FiveM detection report error', { error: error.message });
        res.status(500).json({ error: 'Failed to record detection' });
    }
});

/**
 * POST /api/fivem/sanctions
 * Report sanction from FiveM server
 */
router.post('/sanctions', authenticateFiveM, async (req, res) => {
    try {
        const {
            player_license,
            player_name,
            sanction_type,
            reason,
            duration,
            timestamp
        } = req.body;

        if (!player_license || !sanction_type) {
            return res.status(400).json({ error: 'Missing required fields' });
        }

        let expiresAt = null;
        if (sanction_type === 'ban' && duration && duration > 0) {
            expiresAt = new Date(Date.now() + duration * 1000).toISOString();
        }

        const result = await db.run(`
            INSERT INTO sanctions (player_license, player_name, sanction_type, reason,
                                   duration, expires_at, admin_name, is_active)
            VALUES (?, ?, ?, ?, ?, ?, ?, 1)
        `, [
            player_license,
            player_name || 'Unknown',
            sanction_type,
            reason || 'Anticheat violation',
            duration || 0,
            expiresAt,
            'System (Auto)'
        ]);

        const insertId = result?.lastInsertRowid || result?.insertId;

        // Broadcast to WebSocket clients
        broadcast('sanction', {
            id: insertId,
            player_license,
            player_name,
            sanction_type,
            reason,
            duration
        });

        logger.sanction('sanction_received', {
            server: req.fivemServer.name,
            player: player_license,
            type: sanction_type
        });

        res.status(201).json({ id: insertId });
    } catch (error) {
        logger.error('FiveM sanction report error', { error: error.message });
        res.status(500).json({ error: 'Failed to record sanction' });
    }
});

/**
 * GET /api/fivem/players/:license
 * Get player info for FiveM server
 */
router.get('/players/:license', authenticateFiveM, async (req, res) => {
    try {
        const { license } = req.params;

        const player = await db.queryOne('SELECT * FROM players WHERE license = ?', [license]);

        if (!player) {
            return res.json({ found: false });
        }

        // Check for active ban
        const activeBan = await db.queryOne(`
            SELECT * FROM sanctions
            WHERE player_license = ? AND sanction_type = 'ban' AND is_active = 1
            AND (expires_at IS NULL OR expires_at > CURRENT_TIMESTAMP)
            ORDER BY created_at DESC LIMIT 1
        `, [license]);

        res.json({
            found: true,
            license: player.license,
            name: player.name,
            trust_level: player.trust_level,
            total_detections: player.total_detections,
            banned: !!activeBan,
            ban_reason: activeBan?.reason,
            ban_expires: activeBan?.expires_at
        });
    } catch (error) {
        logger.error('FiveM player lookup error', { error: error.message });
        res.status(500).json({ error: 'Failed to lookup player' });
    }
});

/**
 * POST /api/fivem/players/connect
 * Record player connection
 */
router.post('/players/connect', authenticateFiveM, async (req, res) => {
    try {
        const { license, name, identifiers } = req.body;

        if (!license) {
            return res.status(400).json({ error: 'License required' });
        }

        const existingPlayer = await db.queryOne('SELECT id FROM players WHERE license = ?', [license]);

        if (existingPlayer) {
            await db.run(`
                UPDATE players
                SET last_seen = CURRENT_TIMESTAMP, name = COALESCE(?, name)
                WHERE license = ?
            `, [name, license]);
        } else {
            // Extract identifiers
            let steamId = null, discordId = null;
            if (identifiers) {
                for (const id of identifiers) {
                    if (id.startsWith('steam:')) steamId = id;
                    if (id.startsWith('discord:')) discordId = id;
                }
            }

            await db.run(`
                INSERT INTO players (license, name, steam_id, discord_id)
                VALUES (?, ?, ?, ?)
            `, [license, name || 'Unknown', steamId, discordId]);
        }

        // Broadcast connection
        broadcast('player_connect', { license, name });

        res.json({ success: true });
    } catch (error) {
        logger.error('FiveM player connect error', { error: error.message });
        res.status(500).json({ error: 'Failed to record connection' });
    }
});

/**
 * POST /api/fivem/players/disconnect
 * Record player disconnection
 */
router.post('/players/disconnect', authenticateFiveM, async (req, res) => {
    try {
        const { license, reason } = req.body;

        if (!license) {
            return res.status(400).json({ error: 'License required' });
        }

        await db.run(`
            UPDATE players SET last_seen = CURRENT_TIMESTAMP WHERE license = ?
        `, [license]);

        // Broadcast disconnection
        broadcast('player_disconnect', { license, reason });

        res.json({ success: true });
    } catch (error) {
        logger.error('FiveM player disconnect error', { error: error.message });
        res.status(500).json({ error: 'Failed to record disconnection' });
    }
});

module.exports = { router, setWsClients, broadcast };
