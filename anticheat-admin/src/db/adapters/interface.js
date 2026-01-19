/**
 * Database Adapter Interface
 * Defines the contract that all database adapters must implement
 */

/**
 * @typedef {Object} QueryResult
 * @property {number} lastInsertRowid - The ID of the last inserted row
 * @property {number} changes - Number of rows affected
 */

/**
 * @interface DatabaseAdapter
 * 
 * All database adapters must implement these methods:
 * 
 * @method init() - Initialize the database connection
 * @method close() - Close the database connection
 * @method query(sql, params) - Execute a SELECT query, returns array of objects
 * @method queryOne(sql, params) - Execute a SELECT query, returns single object or null
 * @method run(sql, params) - Execute INSERT/UPDATE/DELETE, returns QueryResult
 * @method transaction(fn) - Execute function within a transaction
 * @method getType() - Returns the adapter type ('sqlite' | 'mysql')
 */

class DatabaseAdapterInterface {
    /**
     * Initialize the database connection
     * @returns {Promise<void>}
     */
    async init() {
        throw new Error('Method init() must be implemented');
    }

    /**
     * Close the database connection
     * @returns {Promise<void>}
     */
    async close() {
        throw new Error('Method close() must be implemented');
    }

    /**
     * Execute a SELECT query
     * @param {string} sql - SQL query string
     * @param {Array} params - Query parameters
     * @returns {Promise<Array<Object>>} Array of row objects
     */
    async query(sql, params = []) {
        throw new Error('Method query() must be implemented');
    }

    /**
     * Execute a SELECT query and return first row
     * @param {string} sql - SQL query string
     * @param {Array} params - Query parameters
     * @returns {Promise<Object|null>} Single row object or null
     */
    async queryOne(sql, params = []) {
        throw new Error('Method queryOne() must be implemented');
    }

    /**
     * Execute an INSERT/UPDATE/DELETE statement
     * @param {string} sql - SQL statement
     * @param {Array} params - Statement parameters
     * @returns {Promise<QueryResult>} Result with lastInsertRowid and changes
     */
    async run(sql, params = []) {
        throw new Error('Method run() must be implemented');
    }

    /**
     * Execute a function within a transaction
     * @param {Function} fn - Async function to execute
     * @returns {Promise<any>} Result of the function
     */
    async transaction(fn) {
        throw new Error('Method transaction() must be implemented');
    }

    /**
     * Get the adapter type
     * @returns {string} 'sqlite' or 'mysql'
     */
    getType() {
        throw new Error('Method getType() must be implemented');
    }

    /**
     * Check if connected
     * @returns {boolean}
     */
    isConnected() {
        throw new Error('Method isConnected() must be implemented');
    }
}

module.exports = { DatabaseAdapterInterface };

