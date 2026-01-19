/**
 * Sanctions Routes
 */

const express = require('express');
const db = require('../db');
const { authenticateToken, requirePermission } = require('../middleware/auth');
const logger = require('../services/logger');

const router = express.Router();

// All routes require authentication
router.use(authenticateToken);

/**
 * GET /api/sanctions
 * List sanctions with filtering
 */
router.get('/', requirePermission('sanctions.view'), async (req, res) => {
    try {
        const {
            page = 1,
            limit = 25,
            type = '',
            active = '',
            player = ''
        } = req.query;

        const offset = (parseInt(page) - 1) * parseInt(limit);
        const conditions = [];
        const params = [];

        if (type) {
            conditions.push('sanction_type = ?');
            params.push(type);
        }
        if (active !== '') {
            conditions.push('is_active = ?');
            params.push(active === 'true' ? 1 : 0);
        }
        if (player) {
            conditions.push('(player_license LIKE ? OR player_name LIKE ?)');
            params.push(`%${player}%`, `%${player}%`);
        }

        const whereClause = conditions.length > 0
            ? 'WHERE ' + conditions.join(' AND ')
            : '';

        const countResult = await db.queryOne(
            `SELECT COUNT(*) as total FROM sanctions ${whereClause}`,
            params
        );

        const limitNum = Number(limit) || 25;
        const offsetNum = Number(offset) || 0;

        const sanctions = await db.query(
            `SELECT * FROM sanctions ${whereClause}
             ORDER BY created_at DESC
             LIMIT ? OFFSET ?`,
            [...params, limitNum, offsetNum]
        );

        res.json({
            sanctions: sanctions || [],
            pagination: {
                page: parseInt(page),
                limit: parseInt(limit),
                total: countResult?.total || 0,
                totalPages: Math.ceil((countResult?.total || 0) / parseInt(limit))
            }
        });
    } catch (error) {
        logger.error('Sanctions list error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch sanctions' });
    }
});

/**
 * POST /api/sanctions
 * Create manual sanction
 */
router.post('/', requirePermission('sanctions.create'), async (req, res) => {
    try {
        const { player_license, player_name, sanction_type, reason, duration } = req.body;

        if (!player_license || !sanction_type || !reason) {
            return res.status(400).json({ error: 'Missing required fields' });
        }

        const validTypes = ['warn', 'kick', 'ban'];
        if (!validTypes.includes(sanction_type)) {
            return res.status(400).json({ error: 'Invalid sanction type' });
        }

        let expiresAt = null;
        if (sanction_type === 'ban' && duration && duration > 0) {
            expiresAt = new Date(Date.now() + duration * 1000).toISOString();
        }

        const result = await db.run(`
            INSERT INTO sanctions (player_license, player_name, sanction_type, reason,
                                   duration, expires_at, admin_id, admin_name, is_active)
            VALUES (?, ?, ?, ?, ?, ?, ?, ?, 1)
        `, [
            player_license,
            player_name || 'Unknown',
            sanction_type,
            reason,
            duration || 0,
            expiresAt,
            req.admin.id,
            req.admin.username
        ]);

        // Update player total detections
        await db.run(`
            UPDATE players SET total_detections = total_detections + 1,
                               updated_at = CURRENT_TIMESTAMP
            WHERE license = ?
        `, [player_license]);

        const insertId = result?.lastInsertRowid || result?.insertId;

        logger.sanction('sanction_created', {
            sanction_id: insertId,
            player: player_license,
            type: sanction_type,
            admin: req.admin.username
        });

        logger.audit('sanction_created', req.admin.id, 'sanction', insertId, {
            player_license,
            sanction_type,
            reason
        });

        res.status(201).json({
            id: insertId,
            message: 'Sanction created'
        });
    } catch (error) {
        logger.error('Sanction create error', { error: error.message });
        res.status(500).json({ error: 'Failed to create sanction' });
    }
});

/**
 * PATCH /api/sanctions/:id
 * Update sanction (e.g., lift ban)
 */
router.patch('/:id', requirePermission('sanctions.edit'), async (req, res) => {
    try {
        const { id } = req.params;
        const { is_active, reason } = req.body;

        const sanction = await db.queryOne('SELECT * FROM sanctions WHERE id = ?', [id]);
        if (!sanction) {
            return res.status(404).json({ error: 'Sanction not found' });
        }

        const updates = [];
        const params = [];

        if (is_active !== undefined) {
            updates.push('is_active = ?');
            params.push(is_active ? 1 : 0);
        }
        if (reason) {
            updates.push('reason = ?');
            params.push(reason);
        }

        if (updates.length === 0) {
            return res.status(400).json({ error: 'No valid fields to update' });
        }

        updates.push('updated_at = CURRENT_TIMESTAMP');
        params.push(id);

        await db.run(
            `UPDATE sanctions SET ${updates.join(', ')} WHERE id = ?`,
            params
        );

        logger.sanction('sanction_updated', {
            sanction_id: id,
            updates: req.body,
            admin: req.admin.username
        });

        logger.audit('sanction_updated', req.admin.id, 'sanction', id, req.body);

        res.json({ message: 'Sanction updated' });
    } catch (error) {
        logger.error('Sanction update error', { error: error.message });
        res.status(500).json({ error: 'Failed to update sanction' });
    }
});

/**
 * DELETE /api/sanctions/:id
 * Delete sanction (soft delete by setting is_active = 0)
 */
router.delete('/:id', requirePermission('sanctions.delete'), async (req, res) => {
    try {
        const { id } = req.params;

        const sanction = await db.queryOne('SELECT * FROM sanctions WHERE id = ?', [id]);
        if (!sanction) {
            return res.status(404).json({ error: 'Sanction not found' });
        }

        await db.run('UPDATE sanctions SET is_active = 0, updated_at = CURRENT_TIMESTAMP WHERE id = ?', [id]);

        logger.audit('sanction_deleted', req.admin.id, 'sanction', id);

        res.json({ message: 'Sanction removed' });
    } catch (error) {
        logger.error('Sanction delete error', { error: error.message });
        res.status(500).json({ error: 'Failed to delete sanction' });
    }
});

module.exports = router;
