/**
 * Stats/Dashboard Routes
 */

const express = require('express');
const db = require('../db');
const { authenticateToken, requirePermission } = require('../middleware/auth');
const logger = require('../services/logger');

const router = express.Router();

router.use(authenticateToken);

/**
 * GET /api/stats/dashboard
 * Get dashboard overview statistics
 */
router.get('/dashboard', requirePermission('dashboard.view'), async (req, res) => {
    try {
        // Total players
        const totalPlayers = await db.queryOne('SELECT COUNT(*) as count FROM players');

        // Active bans
        const activeBans = await db.queryOne(`
            SELECT COUNT(*) as count FROM sanctions
            WHERE sanction_type = 'ban' AND is_active = 1
            AND (expires_at IS NULL OR expires_at > CURRENT_TIMESTAMP)
        `);

        // Today's detections
        const todayStart = new Date();
        todayStart.setHours(0, 0, 0, 0);
        const todayDetections = await db.queryOne(`
            SELECT COUNT(*) as count FROM detections
            WHERE timestamp >= ?
        `, [todayStart.toISOString()]);

        // This week's detections
        const weekStart = new Date();
        weekStart.setDate(weekStart.getDate() - 7);
        const weekDetections = await db.queryOne(`
            SELECT COUNT(*) as count FROM detections
            WHERE timestamp >= ?
        `, [weekStart.toISOString()]);

        // Recent activity (last 10 detections)
        const recentDetections = await db.query(`
            SELECT id, player_name, detection_type, severity, timestamp
            FROM detections
            ORDER BY timestamp DESC
            LIMIT 10
        `);

        // Recent sanctions
        const recentSanctions = await db.query(`
            SELECT id, player_name, sanction_type, reason, created_at
            FROM sanctions
            ORDER BY created_at DESC
            LIMIT 10
        `);

        // Detection trend (last 7 days)
        const detectionTrend = await db.query(`
            SELECT DATE(timestamp) as date, COUNT(*) as count
            FROM detections
            WHERE timestamp >= ?
            GROUP BY DATE(timestamp)
            ORDER BY date ASC
        `, [weekStart.toISOString()]);

        // Active ruleset
        const activeRuleset = await db.queryOne(`
            SELECT id, name, version, updated_at FROM rulesets WHERE is_active = 1
        `);

        res.json({
            overview: {
                totalPlayers: totalPlayers?.count || 0,
                activeBans: activeBans?.count || 0,
                todayDetections: todayDetections?.count || 0,
                weekDetections: weekDetections?.count || 0
            },
            recentDetections: recentDetections || [],
            recentSanctions: recentSanctions || [],
            detectionTrend: detectionTrend || [],
            activeRuleset
        });
    } catch (error) {
        logger.error('Dashboard stats error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch dashboard stats' });
    }
});

/**
 * GET /api/stats/detection-types
 * Get breakdown of detection types
 */
router.get('/detection-types', requirePermission('dashboard.view'), async (req, res) => {
    try {
        const { days = 30 } = req.query;
        const startDate = new Date();
        startDate.setDate(startDate.getDate() - parseInt(days));

        const types = await db.query(`
            SELECT detection_type, COUNT(*) as count
            FROM detections
            WHERE timestamp >= ?
            GROUP BY detection_type
            ORDER BY count DESC
        `, [startDate.toISOString()]);

        res.json(types || []);
    } catch (error) {
        logger.error('Detection types stats error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch detection types' });
    }
});

module.exports = router;
