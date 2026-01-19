-- FiveM Anticheat System
-- A comprehensive server-side anticheat with web admin integration

fx_version 'cerulean'
game 'gta5'

author 'Anticheat System'
description 'Comprehensive anticheat system with web admin dashboard and ELK Stack logging'
version '1.0.0'

-- Shared configuration (loaded first)
shared_scripts {
    'shared/config.lua'
}

-- Server-side scripts
server_scripts {
    'server/logger.lua',
    'server/api.lua',
    'server/detections/speed.lua',
    'server/detections/health.lua',
    'server/detections/weapons.lua',
    'server/detections/entities.lua',
    'server/detections/events.lua',
    'server/detections/resources.lua',
    'server/main.lua'
}

-- Client-side scripts
client_scripts {
    'client/main.lua'
}

-- Dependencies
dependencies {
    '/server:5181', -- Minimum server version for modern APIs
}
