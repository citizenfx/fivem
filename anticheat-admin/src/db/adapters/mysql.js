/**
 * MySQL Database Adapter
 * Uses mysql2 with connection pooling for production-ready MySQL support
 */

const mysql = require('mysql2/promise');

class MySQLAdapter {
    constructor() {
        this.pool = null;
        this.config = {
            host: process.env.MYSQL_HOST || 'localhost',
            port: parseInt(process.env.MYSQL_PORT) || 3306,
            user: process.env.MYSQL_USER || 'anticheat',
            password: process.env.MYSQL_PASSWORD || '',
            database: process.env.MYSQL_DATABASE || 'anticheat_admin',
            waitForConnections: true,
            connectionLimit: parseInt(process.env.MYSQL_POOL_SIZE) || 10,
            queueLimit: 0,
            enableKeepAlive: true,
            keepAliveInitialDelay: 0
        };
    }

    async init() {
        if (this.pool) return;

        try {
            this.pool = mysql.createPool(this.config);
            
            // Test connection
            const connection = await this.pool.getConnection();
            console.log('MySQL connected successfully');
            connection.release();
        } catch (error) {
            console.error('MySQL connection failed:', error.message);
            throw error;
        }
    }

    async close() {
        if (this.pool) {
            await this.pool.end();
            this.pool = null;
        }
    }

    /**
     * Normalize parameters for MySQL prepared statements
     * MySQL execute() requires correct types - especially for LIMIT/OFFSET
     */
    normalizeParams(params) {
        if (!params || !Array.isArray(params)) return [];
        return params.map(param => {
            // Handle null/undefined
            if (param === null || param === undefined) {
                return null;
            }
            // Convert numeric strings to integers for LIMIT/OFFSET compatibility
            if (typeof param === 'string' && /^-?\d+$/.test(param)) {
                return parseInt(param, 10);
            }
            // Ensure numbers are actual numbers (not NaN)
            if (typeof param === 'number' && isNaN(param)) {
                return 0;
            }
            return param;
        });
    }

    async query(sql, params = []) {
        if (!this.pool) throw new Error('Database not initialized');

        try {
            const normalizedParams = this.normalizeParams(params);
            // Use query() instead of execute() for better compatibility with dynamic SQL
            // query() handles parameter binding more flexibly
            const [rows] = await this.pool.query(sql, normalizedParams);
            return rows;
        } catch (error) {
            console.error('MySQL query error:', sql, error);
            throw error;
        }
    }

    async queryOne(sql, params = []) {
        const results = await this.query(sql, params);
        return results[0] || null;
    }

    async run(sql, params = []) {
        if (!this.pool) throw new Error('Database not initialized');

        try {
            const normalizedParams = this.normalizeParams(params);
            // Use query() for better compatibility with dynamic SQL
            const [result] = await this.pool.query(sql, normalizedParams);
            return {
                lastInsertRowid: result.insertId,
                changes: result.affectedRows
            };
        } catch (error) {
            console.error('MySQL run error:', sql, error);
            throw error;
        }
    }

    async transaction(fn) {
        if (!this.pool) throw new Error('Database not initialized');

        const connection = await this.pool.getConnection();

        try {
            await connection.beginTransaction();

            // Create a scoped adapter for the transaction
            const self = this;
            const scopedAdapter = {
                query: async (sql, params = []) => {
                    const normalizedParams = self.normalizeParams(params);
                    // Use query() for better compatibility
                    const [rows] = await connection.query(sql, normalizedParams);
                    return rows;
                },
                queryOne: async (sql, params = []) => {
                    const normalizedParams = self.normalizeParams(params);
                    const [rows] = await connection.query(sql, normalizedParams);
                    return rows[0] || null;
                },
                run: async (sql, params = []) => {
                    const normalizedParams = self.normalizeParams(params);
                    const [result] = await connection.query(sql, normalizedParams);
                    return {
                        lastInsertRowid: result.insertId,
                        changes: result.affectedRows
                    };
                }
            };
            
            const result = await fn(scopedAdapter);
            await connection.commit();
            return result;
        } catch (error) {
            await connection.rollback();
            throw error;
        } finally {
            connection.release();
        }
    }

    getType() {
        return 'mysql';
    }

    isConnected() {
        return this.pool !== null;
    }
}

module.exports = { MySQLAdapter };

