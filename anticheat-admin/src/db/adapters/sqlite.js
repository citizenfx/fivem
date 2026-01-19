/**
 * SQLite Database Adapter
 * Uses sql.js (pure JavaScript SQLite implementation)
 */

const initSqlJs = require('sql.js');
const fs = require('fs');
const path = require('path');

const DB_PATH = process.env.DB_PATH || path.join(__dirname, '../../../data/anticheat.db');

class SQLiteAdapter {
    constructor() {
        this.db = null;
        this.SQL = null;
        this.inTransaction = false;
        this.saveTimeout = null;
    }

    async init() {
        if (this.db) return;

        this.SQL = await initSqlJs();
        
        // Ensure data directory exists
        const dataDir = path.dirname(DB_PATH);
        if (!fs.existsSync(dataDir)) {
            fs.mkdirSync(dataDir, { recursive: true });
        }

        // Load existing database or create new one
        if (fs.existsSync(DB_PATH)) {
            const fileBuffer = fs.readFileSync(DB_PATH);
            this.db = new this.SQL.Database(fileBuffer);
        } else {
            this.db = new this.SQL.Database();
        }

        // Setup exit handlers
        this._setupExitHandlers();
    }

    _setupExitHandlers() {
        const saveAndExit = () => {
            this.saveSync();
            process.exit(0);
        };
        
        process.on('exit', () => this.saveSync());
        process.on('SIGINT', saveAndExit);
        process.on('SIGTERM', saveAndExit);
    }

    async close() {
        if (this.saveTimeout) {
            clearTimeout(this.saveTimeout);
        }
        this.saveSync();
        if (this.db) {
            this.db.close();
            this.db = null;
        }
    }

    // Debounced save - prevents blocking on every write
    save() {
        if (this.saveTimeout) {
            clearTimeout(this.saveTimeout);
        }
        this.saveTimeout = setTimeout(() => {
            this.saveSync();
        }, 100); // Save after 100ms of no writes
    }

    saveSync() {
        if (!this.db) return;
        try {
            const data = this.db.export();
            const buffer = Buffer.from(data);
            fs.writeFileSync(DB_PATH, buffer);
        } catch (error) {
            console.error('SQLite save error:', error);
        }
    }

    async query(sql, params = []) {
        if (!this.db) throw new Error('Database not initialized');
        
        try {
            const stmt = this.db.prepare(sql);
            if (params.length) stmt.bind(params);
            
            const results = [];
            while (stmt.step()) {
                results.push(stmt.getAsObject());
            }
            stmt.free();
            return results;
        } catch (error) {
            console.error('SQLite query error:', sql, error);
            throw error;
        }
    }

    async queryOne(sql, params = []) {
        const results = await this.query(sql, params);
        return results[0] || null;
    }

    async run(sql, params = []) {
        if (!this.db) throw new Error('Database not initialized');
        
        try {
            this.db.run(sql, params);
            
            // Debounced save after writes (if not in transaction)
            if (!this.inTransaction) {
                this.save();
            }
            
            // Get last insert rowid
            const lastId = this.db.exec('SELECT last_insert_rowid() as id')[0]?.values[0]?.[0];
            return { lastInsertRowid: lastId, changes: this.db.getRowsModified() };
        } catch (error) {
            console.error('SQLite run error:', sql, error);
            throw error;
        }
    }

    async transaction(fn) {
        if (this.inTransaction) {
            return await fn();
        }
        
        this.inTransaction = true;
        try {
            this.db.exec('BEGIN TRANSACTION');
            const result = await fn();
            this.db.exec('COMMIT');
            this.inTransaction = false;
            this.save();
            return result;
        } catch (error) {
            try {
                this.db.exec('ROLLBACK');
            } catch (rollbackError) {
                console.error('SQLite rollback failed:', rollbackError.message);
            }
            this.inTransaction = false;
            throw error;
        }
    }

    getType() {
        return 'sqlite';
    }

    isConnected() {
        return this.db !== null;
    }
}

module.exports = { SQLiteAdapter };

