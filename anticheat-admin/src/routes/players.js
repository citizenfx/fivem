/**
 * Players Routes
 */

const express = require('express');
const db = require('../db');
const { authenticateToken, requirePermission } = require('../middleware/auth');
const logger = require('../services/logger');

const router = express.Router();

// All routes require authentication
router.use(authenticateToken);

/**
 * GET /api/players
 * List players with pagination and search
 */
router.get('/', requirePermission('players.view'), async (req, res) => {
    try {
        const {
            page = 1,
            limit = 25,
            search = '',
            sort = 'last_seen',
            order = 'DESC'
        } = req.query;

        const offset = (parseInt(page) - 1) * parseInt(limit);
        const validSorts = ['last_seen', 'first_seen', 'name', 'total_detections'];
        const sortColumn = validSorts.includes(sort) ? sort : 'last_seen';
        const sortOrder = order.toUpperCase() === 'ASC' ? 'ASC' : 'DESC';

        let whereClause = '';
        let params = [];

        if (search) {
            whereClause = 'WHERE name LIKE ? OR license LIKE ?';
            params = [`%${search}%`, `%${search}%`];
        }

        const countResult = await db.queryOne(
            `SELECT COUNT(*) as total FROM players ${whereClause}`,
            params
        );

        const limitNum = Number(limit) || 25;
        const offsetNum = Number(offset) || 0;

        const players = await db.query(
            `SELECT id, license, name, steam_id, discord_id, first_seen, last_seen,
                    trust_level, total_detections, notes, created_at
             FROM players ${whereClause}
             ORDER BY ${sortColumn} ${sortOrder}
             LIMIT ? OFFSET ?`,
            [...params, limitNum, offsetNum]
        );

        res.json({
            players: players || [],
            pagination: {
                page: parseInt(page),
                limit: parseInt(limit),
                total: countResult?.total || 0,
                totalPages: Math.ceil((countResult?.total || 0) / parseInt(limit))
            }
        });
    } catch (error) {
        logger.error('Players list error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch players' });
    }
});

/**
 * GET /api/players/:license
 * Get single player by license
 */
router.get('/:license', requirePermission('players.view'), async (req, res) => {
    try {
        const { license } = req.params;

        const player = await db.queryOne(
            'SELECT * FROM players WHERE license = ?',
            [license]
        );

        if (!player) {
            return res.status(404).json({ error: 'Player not found' });
        }

        // Get recent detections
        const detections = await db.query(
            `SELECT * FROM detections WHERE player_license = ?
             ORDER BY timestamp DESC LIMIT 50`,
            [license]
        );

        // Get sanctions
        const sanctions = await db.query(
            `SELECT * FROM sanctions WHERE player_license = ?
             ORDER BY created_at DESC`,
            [license]
        );

        res.json({
            ...player,
            detections: detections || [],
            sanctions: sanctions || []
        });
    } catch (error) {
        logger.error('Player detail error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch player' });
    }
});

/**
 * PATCH /api/players/:license
 * Update player notes/trust level
 */
router.patch('/:license', requirePermission('players.edit'), async (req, res) => {
    try {
        const { license } = req.params;
        const { notes, trust_level } = req.body;

        const player = await db.queryOne('SELECT * FROM players WHERE license = ?', [license]);
        if (!player) {
            return res.status(404).json({ error: 'Player not found' });
        }

        const updates = [];
        const params = [];

        if (notes !== undefined) {
            updates.push('notes = ?');
            params.push(notes);
        }
        if (trust_level !== undefined) {
            updates.push('trust_level = ?');
            params.push(trust_level);
        }

        if (updates.length === 0) {
            return res.status(400).json({ error: 'No valid fields to update' });
        }

        updates.push('updated_at = CURRENT_TIMESTAMP');
        params.push(license);

        await db.run(
            `UPDATE players SET ${updates.join(', ')} WHERE license = ?`,
            params
        );

        logger.audit('player_updated', req.admin.id, 'player', player.id, { license, updates: req.body });

        res.json({ message: 'Player updated' });
    } catch (error) {
        logger.error('Player update error', { error: error.message });
        res.status(500).json({ error: 'Failed to update player' });
    }
});

module.exports = router;
