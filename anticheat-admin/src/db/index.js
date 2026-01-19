/**
 * Database Connection Module
 * Factory pattern supporting both SQLite and MySQL backends
 */

const { SQLiteAdapter } = require('./adapters/sqlite');

// Lazy-load MySQL adapter only when needed
let MySQLAdapter = null;

const DB_TYPE = (process.env.DB_TYPE || 'sqlite').toLowerCase();

let adapter = null;

/**
 * Initialize the database with the appropriate adapter
 * @returns {Promise<void>}
 */
async function initDb() {
    if (adapter && adapter.isConnected()) return adapter;

    if (DB_TYPE === 'mysql') {
        // Lazy load MySQL adapter
        if (!MySQLAdapter) {
            MySQLAdapter = require('./adapters/mysql').MySQLAdapter;
        }
        adapter = new MySQLAdapter();
    } else {
        adapter = new SQLiteAdapter();
    }

    await adapter.init();
    console.log(`Database initialized: ${adapter.getType()}`);
    return adapter;
}

/**
 * Close database connection
 */
async function closeDb() {
    if (adapter) {
        await adapter.close();
        adapter = null;
    }
}

/**
 * Execute SELECT query - returns array of objects
 * @param {string} sql
 * @param {Array} params
 * @returns {Array<Object>}
 */
function query(sql, params = []) {
    if (!adapter) throw new Error('Database not initialized');
    // For backwards compatibility, keep sync API but adapter is async
    // The adapter methods are designed to be fast for sync-like usage
    return adapter.query(sql, params);
}

/**
 * Execute SELECT query - returns single object or null
 * @param {string} sql
 * @param {Array} params
 * @returns {Object|null}
 */
function queryOne(sql, params = []) {
    if (!adapter) throw new Error('Database not initialized');
    return adapter.queryOne(sql, params);
}

/**
 * Execute INSERT/UPDATE/DELETE
 * @param {string} sql
 * @param {Array} params
 * @returns {{lastInsertRowid: number, changes: number}}
 */
function run(sql, params = []) {
    if (!adapter) throw new Error('Database not initialized');
    return adapter.run(sql, params);
}

/**
 * Execute function within a transaction
 * @param {Function} fn
 * @returns {any}
 */
function transaction(fn) {
    if (!adapter) throw new Error('Database not initialized');
    return adapter.transaction(fn);
}

/**
 * Save database (only applicable for SQLite)
 */
function saveDb() {
    if (adapter && adapter.getType() === 'sqlite' && adapter.saveSync) {
        adapter.saveSync();
    }
}

/**
 * Get the database adapter type
 * @returns {string} 'sqlite' or 'mysql'
 */
function getDbType() {
    return adapter ? adapter.getType() : DB_TYPE;
}

/**
 * Get the adapter instance (for advanced usage)
 * @returns {Object}
 */
function getAdapter() {
    return adapter;
}

module.exports = {
    initDb,
    closeDb,
    query,
    queryOne,
    run,
    transaction,
    saveDb,
    getDbType,
    getAdapter
};
