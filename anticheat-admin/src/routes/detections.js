/**
 * Detections Routes
 */

const express = require('express');
const db = require('../db');
const { authenticateToken, requirePermission } = require('../middleware/auth');
const logger = require('../services/logger');

const router = express.Router();

// All routes require authentication
router.use(authenticateToken);

/**
 * GET /api/detections
 * List detections with filtering and pagination
 */
router.get('/', requirePermission('detections.view'), async (req, res) => {
    try {
        const {
            page = 1,
            limit = 50,
            type = '',
            severity = '',
            player = '',
            startDate = '',
            endDate = ''
        } = req.query;

        const offset = (parseInt(page) - 1) * parseInt(limit);
        const conditions = [];
        const params = [];

        if (type) {
            conditions.push('detection_type = ?');
            params.push(type);
        }
        if (severity) {
            conditions.push('severity = ?');
            params.push(severity);
        }
        if (player) {
            conditions.push('(player_license LIKE ? OR player_name LIKE ?)');
            params.push(`%${player}%`, `%${player}%`);
        }
        if (startDate) {
            conditions.push('timestamp >= ?');
            params.push(startDate);
        }
        if (endDate) {
            conditions.push('timestamp <= ?');
            params.push(endDate);
        }

        const whereClause = conditions.length > 0
            ? 'WHERE ' + conditions.join(' AND ')
            : '';

        const countResult = await db.queryOne(
            `SELECT COUNT(*) as total FROM detections ${whereClause}`,
            params
        );

        const limitNum = Number(limit) || 25;
        const offsetNum = Number(offset) || 0;

        const detections = await db.query(
            `SELECT * FROM detections ${whereClause}
             ORDER BY timestamp DESC
             LIMIT ? OFFSET ?`,
            [...params, limitNum, offsetNum]
        );

        // Parse JSON data field
        const parsedDetections = (detections || []).map(d => ({
            ...d,
            data: d.data ? (typeof d.data === 'string' ? JSON.parse(d.data) : d.data) : null
        }));

        res.json({
            detections: parsedDetections,
            pagination: {
                page: parseInt(page),
                limit: parseInt(limit),
                total: countResult?.total || 0,
                totalPages: Math.ceil((countResult?.total || 0) / parseInt(limit))
            }
        });
    } catch (error) {
        logger.error('Detections list error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch detections' });
    }
});

/**
 * GET /api/detections/stats
 * Get detection statistics
 */
router.get('/stats', requirePermission('detections.view'), async (req, res) => {
    try {
        const { days = 7 } = req.query;
        const startDate = new Date();
        startDate.setDate(startDate.getDate() - parseInt(days));

        // Overall counts by type
        const byType = await db.query(`
            SELECT detection_type, COUNT(*) as count
            FROM detections
            WHERE timestamp >= ?
            GROUP BY detection_type
            ORDER BY count DESC
        `, [startDate.toISOString()]);

        // By severity
        const bySeverity = await db.query(`
            SELECT severity, COUNT(*) as count
            FROM detections
            WHERE timestamp >= ?
            GROUP BY severity
        `, [startDate.toISOString()]);

        // Daily trend
        const dailyTrend = await db.query(`
            SELECT DATE(timestamp) as date, COUNT(*) as count
            FROM detections
            WHERE timestamp >= ?
            GROUP BY DATE(timestamp)
            ORDER BY date ASC
        `, [startDate.toISOString()]);

        // Top flagged players
        const topPlayers = await db.query(`
            SELECT player_license, player_name, COUNT(*) as detection_count
            FROM detections
            WHERE timestamp >= ?
            GROUP BY player_license
            ORDER BY detection_count DESC
            LIMIT 10
        `, [startDate.toISOString()]);

        res.json({
            byType: byType || [],
            bySeverity: bySeverity || [],
            dailyTrend: dailyTrend || [],
            topPlayers: topPlayers || [],
            period: {
                days: parseInt(days),
                startDate: startDate.toISOString()
            }
        });
    } catch (error) {
        logger.error('Detection stats error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch statistics' });
    }
});

/**
 * GET /api/detections/:id
 * Get single detection
 */
router.get('/:id', requirePermission('detections.view'), async (req, res) => {
    try {
        const detection = await db.queryOne(
            'SELECT * FROM detections WHERE id = ?',
            [req.params.id]
        );

        if (!detection) {
            return res.status(404).json({ error: 'Detection not found' });
        }

        res.json({
            ...detection,
            data: detection.data ? (typeof detection.data === 'string' ? JSON.parse(detection.data) : detection.data) : null
        });
    } catch (error) {
        logger.error('Detection detail error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch detection' });
    }
});

module.exports = router;
