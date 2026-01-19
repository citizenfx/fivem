/**
 * Rulesets Routes
 */

const express = require('express');
const db = require('../db');
const { authenticateToken, requirePermission } = require('../middleware/auth');
const logger = require('../services/logger');

const router = express.Router();

// All routes require authentication
router.use(authenticateToken);

/**
 * GET /api/rulesets
 * List all rulesets
 */
router.get('/', requirePermission('rulesets.view'), async (req, res) => {
    try {
        const rulesets = await db.query(`
            SELECT id, name, description, is_active, version, created_by, created_at, updated_at
            FROM rulesets
            ORDER BY is_active DESC, updated_at DESC
        `);

        // Convert is_active to proper boolean
        const result = (rulesets || []).map(r => ({
            ...r,
            is_active: r.is_active === 1 || r.is_active === true
        }));

        res.json(result);
    } catch (error) {
        logger.error('Rulesets list error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch rulesets' });
    }
});

/**
 * GET /api/rulesets/active
 * Get currently active ruleset
 */
router.get('/active', requirePermission('rulesets.view'), async (req, res) => {
    try {
        const ruleset = await db.queryOne(
            'SELECT * FROM rulesets WHERE is_active = 1'
        );

        if (!ruleset) {
            return res.status(404).json({ error: 'No active ruleset' });
        }

        const configJson = ruleset.config_json;
        const config = typeof configJson === 'string' ? JSON.parse(configJson) : configJson;

        res.json({
            id: ruleset.id,
            name: ruleset.name,
            description: ruleset.description,
            version: ruleset.version,
            ...config
        });
    } catch (error) {
        logger.error('Active ruleset error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch active ruleset' });
    }
});

/**
 * GET /api/rulesets/:id
 * Get single ruleset
 */
router.get('/:id', requirePermission('rulesets.view'), async (req, res) => {
    try {
        const ruleset = await db.queryOne(
            'SELECT * FROM rulesets WHERE id = ?',
            [req.params.id]
        );

        if (!ruleset) {
            return res.status(404).json({ error: 'Ruleset not found' });
        }

        const configJson = ruleset.config_json;
        const config = typeof configJson === 'string' ? JSON.parse(configJson) : configJson;

        res.json({
            ...ruleset,
            is_active: ruleset.is_active === 1 || ruleset.is_active === true,
            config
        });
    } catch (error) {
        logger.error('Ruleset detail error', { error: error.message });
        res.status(500).json({ error: 'Failed to fetch ruleset' });
    }
});

/**
 * POST /api/rulesets
 * Create new ruleset
 */
router.post('/', requirePermission('rulesets.create'), async (req, res) => {
    try {
        const { name, description, config } = req.body;

        if (!name || !config) {
            return res.status(400).json({ error: 'Name and config required' });
        }

        const configJson = typeof config === 'string' ? config : JSON.stringify(config);

        const result = await db.run(`
            INSERT INTO rulesets (name, description, config_json, created_by)
            VALUES (?, ?, ?, ?)
        `, [name, description || '', configJson, req.admin.id]);

        const insertId = result?.lastInsertRowid || result?.insertId;

        logger.audit('ruleset_created', req.admin.id, 'ruleset', insertId, { name });

        res.status(201).json({
            id: insertId,
            message: 'Ruleset created'
        });
    } catch (error) {
        logger.error('Ruleset create error', { error: error.message });
        res.status(500).json({ error: 'Failed to create ruleset' });
    }
});

/**
 * PUT /api/rulesets/:id
 * Update ruleset
 */
router.put('/:id', requirePermission('rulesets.edit'), async (req, res) => {
    try {
        const { id } = req.params;
        const { name, description, config } = req.body;

        const ruleset = await db.queryOne('SELECT * FROM rulesets WHERE id = ?', [id]);
        if (!ruleset) {
            return res.status(404).json({ error: 'Ruleset not found' });
        }

        const configJson = config
            ? (typeof config === 'string' ? config : JSON.stringify(config))
            : ruleset.config_json;

        await db.run(`
            UPDATE rulesets
            SET name = ?, description = ?, config_json = ?,
                version = version + 1, updated_at = CURRENT_TIMESTAMP
            WHERE id = ?
        `, [
            name || ruleset.name,
            description !== undefined ? description : ruleset.description,
            configJson,
            id
        ]);

        logger.audit('ruleset_updated', req.admin.id, 'ruleset', id, { name });

        res.json({ message: 'Ruleset updated' });
    } catch (error) {
        logger.error('Ruleset update error', { error: error.message });
        res.status(500).json({ error: 'Failed to update ruleset' });
    }
});

/**
 * POST /api/rulesets/:id/activate
 * Activate a ruleset (deactivates others)
 */
router.post('/:id/activate', requirePermission('rulesets.activate'), async (req, res) => {
    try {
        const { id } = req.params;

        const ruleset = await db.queryOne('SELECT * FROM rulesets WHERE id = ?', [id]);
        if (!ruleset) {
            return res.status(404).json({ error: 'Ruleset not found' });
        }

        // Deactivate all, then activate selected
        await db.transaction(async () => {
            await db.run('UPDATE rulesets SET is_active = 0');
            await db.run('UPDATE rulesets SET is_active = 1, updated_at = CURRENT_TIMESTAMP WHERE id = ?', [id]);
        });

        logger.audit('ruleset_activated', req.admin.id, 'ruleset', id, { name: ruleset.name });

        res.json({ message: 'Ruleset activated' });
    } catch (error) {
        logger.error('Ruleset activate error', { error: error.message });
        res.status(500).json({ error: 'Failed to activate ruleset' });
    }
});

/**
 * DELETE /api/rulesets/:id
 * Delete ruleset
 */
router.delete('/:id', requirePermission('rulesets.delete'), async (req, res) => {
    try {
        const { id } = req.params;

        const ruleset = await db.queryOne('SELECT * FROM rulesets WHERE id = ?', [id]);
        if (!ruleset) {
            return res.status(404).json({ error: 'Ruleset not found' });
        }

        if (ruleset.is_active) {
            return res.status(400).json({ error: 'Cannot delete active ruleset' });
        }

        await db.run('DELETE FROM rulesets WHERE id = ?', [id]);

        logger.audit('ruleset_deleted', req.admin.id, 'ruleset', id);

        res.json({ message: 'Ruleset deleted' });
    } catch (error) {
        logger.error('Ruleset delete error', { error: error.message });
        res.status(500).json({ error: 'Failed to delete ruleset' });
    }
});

module.exports = router;
