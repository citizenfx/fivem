/**
 * Database Initialization Script
 * Sets up database schema and default data for SQLite or MySQL
 */

const { initDb, run, query, saveDb, getDbType, closeDb } = require('./index');
const fs = require('fs');
const path = require('path');
const bcrypt = require('bcrypt');
const { v4: uuidv4 } = require('uuid');
const { PERMISSIONS, DEFAULT_ROLES } = require('../services/permissions');

async function initialize() {
    console.log('Initializing database...');

    // Initialize database (SQLite or MySQL based on DB_TYPE env var)
    await initDb();
    const dbType = getDbType();
    console.log(`Using database: ${dbType}`);

    // Read and execute schema based on database type
    const schemaFile = dbType === 'mysql' ? 'schema.mysql.sql' : 'schema.sql';
    const schemaPath = path.join(__dirname, schemaFile);
    const schema = fs.readFileSync(schemaPath, 'utf8');

    // Split by semicolons and execute each statement
    const statements = schema.split(';').filter(s => s.trim());
    for (const stmt of statements) {
        if (stmt.trim()) {
            try {
                await run(stmt);
            } catch (error) {
                // Ignore "table already exists" errors
                if (!error.message.includes('already exists') &&
                    !error.message.includes('Duplicate')) {
                    console.error('Schema error:', error.message);
                }
            }
        }
    }

    console.log('Schema applied successfully.');

    // Seed permissions
    await seedPermissions();

    // Seed default roles
    await seedRoles();

    // Check if admin user exists
    const existingAdmin = await query('SELECT id FROM admins WHERE username = ?', ['admin']);

    if (!existingAdmin || existingAdmin.length === 0) {
        // Get superadmin role ID
        const superadminRole = await query('SELECT id FROM roles WHERE name = ?', ['superadmin']);
        const roleId = superadminRole?.[0]?.id || 1;

        // Create default admin user
        const defaultPassword = process.env.ADMIN_PASSWORD || 'changeme123';
        const passwordHash = await bcrypt.hash(defaultPassword, 10);

        await run(`
            INSERT INTO admins (username, password_hash, role_id, email)
            VALUES (?, ?, ?, ?)
        `, ['admin', passwordHash, roleId, 'admin@localhost']);

        console.log('');
        console.log('╔════════════════════════════════════════════════════════════╗');
        console.log('║  Default admin user created:                               ║');
        console.log('║    Username: admin                                         ║');
        console.log('║    Password: ' + defaultPassword.padEnd(45) + '║');
        console.log('║    Role: superadmin                                        ║');
        console.log('║                                                            ║');
        console.log('║  ⚠️  IMPORTANT: Change this password after first login!    ║');
        console.log('╚════════════════════════════════════════════════════════════╝');
        console.log('');
    } else {
        console.log('Admin user already exists, skipping...');
    }

    // Check if API key exists
    const existingKey = await query('SELECT id FROM api_keys WHERE name = ?', ['default-fivem']);

    if (!existingKey || existingKey.length === 0) {
        // Create default API key for FiveM server
        const apiKey = uuidv4();
        const keyHash = await bcrypt.hash(apiKey, 10);

        await run(`
            INSERT INTO api_keys (name, key_hash, server_name)
            VALUES (?, ?, ?)
        `, ['default-fivem', keyHash, 'Main FiveM Server']);

        console.log('');
        console.log('╔════════════════════════════════════════════════════════════╗');
        console.log('║  FiveM API Key generated:                                  ║');
        console.log('║                                                            ║');
        console.log('║    ' + apiKey + '                 ║');
        console.log('║                                                            ║');
        console.log('║  Add this to your server.cfg:                              ║');
        console.log('║    set anticheat_api_key "' + apiKey + '"     ║');
        console.log('║                                                            ║');
        console.log('║  ⚠️  Save this key - it cannot be retrieved later!         ║');
        console.log('╚════════════════════════════════════════════════════════════╝');
        console.log('');
    } else {
        console.log('API key already exists, skipping...');
    }

    // Check if default ruleset exists
    const existingRuleset = await query('SELECT id FROM rulesets WHERE name = ?', ['Default']);

    if (!existingRuleset || existingRuleset.length === 0) {
        // Create default ruleset
        const defaultConfig = {
            detections: {
                speed: { enabled: true, maxSpeed: 200, maxVehicleSpeed: 500 },
                health: { enabled: true, maxHealth: 200, maxArmor: 100 },
                weapons: { enabled: true, blacklistEnabled: true },
                entities: { enabled: true, maxPerPlayer: 50 },
                events: { enabled: true, rateLimit: 30 },
                resources: { enabled: true }
            },
            sanctions: {
                autoKickThreshold: 3,
                autoBanThreshold: 5
            }
        };

        await run(`
            INSERT INTO rulesets (name, description, config_json, is_active, created_by)
            VALUES (?, ?, ?, 1, 1)
        `, ['Default', 'Default anticheat ruleset', JSON.stringify(defaultConfig)]);

        console.log('Default ruleset created and activated.');
    } else {
        console.log('Default ruleset already exists, skipping...');
    }

    // Final save (only for SQLite)
    saveDb();

    // Close database connection
    await closeDb();

    console.log('');
    console.log('✅ Database initialization complete!');
    console.log('');
}

/**
 * Seed default permissions
 */
async function seedPermissions() {
    console.log('Seeding permissions...');

    for (const [name, info] of Object.entries(PERMISSIONS)) {
        try {
            const existing = await query('SELECT id FROM permissions WHERE name = ?', [name]);
            if (!existing || existing.length === 0) {
                await run(
                    'INSERT INTO permissions (name, description, category) VALUES (?, ?, ?)',
                    [name, info.description, info.category]
                );
            }
        } catch (error) {
            // Ignore duplicates
            if (!error.message.includes('Duplicate') && !error.message.includes('UNIQUE')) {
                console.error(`Error seeding permission ${name}:`, error.message);
            }
        }
    }

    console.log(`Seeded ${Object.keys(PERMISSIONS).length} permissions.`);
}

/**
 * Seed default roles with their permissions
 */
async function seedRoles() {
    console.log('Seeding roles...');

    for (const [roleName, roleInfo] of Object.entries(DEFAULT_ROLES)) {
        try {
            // Check if role exists
            let roleResult = await query('SELECT id FROM roles WHERE name = ?', [roleName]);
            let roleId;

            if (!roleResult || roleResult.length === 0) {
                // Create role
                const insertResult = await run(
                    'INSERT INTO roles (name, description, is_system) VALUES (?, ?, ?)',
                    [roleName, roleInfo.description, roleInfo.isSystem ? 1 : 0]
                );
                roleId = insertResult?.lastInsertRowid || insertResult?.insertId;
                console.log(`  Created role: ${roleName}`);
            } else {
                roleId = roleResult[0].id;
            }

            // Assign permissions to role
            for (const permName of roleInfo.permissions) {
                try {
                    const permResult = await query('SELECT id FROM permissions WHERE name = ?', [permName]);
                    if (permResult && permResult.length > 0) {
                        const permId = permResult[0].id;

                        // Check if mapping exists
                        const existingMapping = await query(
                            'SELECT id FROM role_permissions WHERE role_id = ? AND permission_id = ?',
                            [roleId, permId]
                        );

                        if (!existingMapping || existingMapping.length === 0) {
                            await run(
                                'INSERT INTO role_permissions (role_id, permission_id) VALUES (?, ?)',
                                [roleId, permId]
                            );
                        }
                    }
                } catch (error) {
                    // Ignore duplicates
                    if (!error.message.includes('Duplicate') && !error.message.includes('UNIQUE')) {
                        console.error(`Error mapping permission ${permName} to ${roleName}:`, error.message);
                    }
                }
            }
        } catch (error) {
            if (!error.message.includes('Duplicate') && !error.message.includes('UNIQUE')) {
                console.error(`Error seeding role ${roleName}:`, error.message);
            }
        }
    }

    console.log(`Seeded ${Object.keys(DEFAULT_ROLES).length} roles.`);
}

initialize().catch(error => {
    console.error('Initialization failed:', error);
    process.exit(1);
});
