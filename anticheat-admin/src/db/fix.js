/**
 * Database Fix Script
 * Fixes issues with is_active field - ensures only one ruleset is active
 */

const { initDb, query, run, saveDb } = require('./index');

async function fixDatabase() {
    console.log('Initializing database...');
    await initDb();
    
    console.log('Checking rulesets...');
    const rulesets = query('SELECT id, name, is_active FROM rulesets ORDER BY id ASC');
    
    console.log('Current rulesets:');
    rulesets.forEach(r => {
        console.log(`  ID: ${r.id}, Name: ${r.name}, is_active: ${r.is_active}`);
    });
    
    // Count active rulesets
    const activeCount = rulesets.filter(r => r.is_active === 1 || r.is_active === true).length;
    console.log(`\nActive rulesets: ${activeCount}`);
    
    if (activeCount !== 1) {
        console.log('\nFixing: Setting all rulesets to inactive, then activating first one...');
        
        // Set all to inactive
        run('UPDATE rulesets SET is_active = 0');
        
        // Activate the first one (or Default if exists)
        const defaultRuleset = query("SELECT id FROM rulesets WHERE name = 'Default' LIMIT 1")[0];
        const firstRuleset = rulesets[0];
        
        const toActivate = defaultRuleset || firstRuleset;
        if (toActivate) {
            run('UPDATE rulesets SET is_active = 1 WHERE id = ?', [toActivate.id]);
            console.log(`Activated ruleset ID: ${toActivate.id}`);
        }
        
        saveDb();
        console.log('Database saved!');
    } else {
        console.log('Database is already correct - exactly one ruleset is active.');
    }
    
    // Verify fix
    console.log('\nVerifying...');
    const after = query('SELECT id, name, is_active FROM rulesets ORDER BY id ASC');
    after.forEach(r => {
        console.log(`  ID: ${r.id}, Name: ${r.name}, is_active: ${r.is_active}`);
    });
    
    console.log('\n✅ Done!');
    process.exit(0);
}

fixDatabase().catch(err => {
    console.error('Fix failed:', err);
    process.exit(1);
});
