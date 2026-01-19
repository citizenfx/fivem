# FiveM Anticheat System

A comprehensive anticheat solution for FiveM roleplay servers with web-based administration and ELK Stack logging integration.

## Features

- **Server-side Detection Modules**
  - Speed/teleportation detection
  - Health/godmode detection
  - Weapon modification detection
  - Entity spawn validation
  - Event validation and rate limiting
  - Resource injection detection

- **Web Admin Dashboard**
  - Real-time monitoring via WebSocket
  - Player management
  - Detection log viewer
  - Ban/kick/warn management
  - Dynamic ruleset configuration

- **ELK Stack Integration**
  - Structured JSON logging
  - Filebeat log collection
  - Logstash processing pipeline
  - Kibana dashboards

## Quick Start

### 1. FiveM Resource Setup

```bash
# Copy resource to your FiveM server
cp -r anticheat-resource /path/to/fivem/server-data/resources/

# Add to server.cfg
echo 'ensure anticheat-resource' >> /path/to/fivem/server-data/server.cfg

# Set API key (optional, for web admin sync)
echo 'set anticheat_api_key "your-api-key-here"' >> /path/to/fivem/server-data/server.cfg
```

### 2. Web Admin Dashboard

```bash
# Install dependencies
cd anticheat-admin
npm install

# Initialize database
npm run init-db

# Start server
npm start

# Access at http://localhost:3000
# Default login: admin / changeme123
```

### 3. Frontend Development

```bash
cd anticheat-admin/frontend
npm install
npm run dev

# Access at http://localhost:5173
```

### 4. ELK Stack (Optional)

See `elk-config/` for configuration files:

- `filebeat.yml` - Log collection configuration
- `logstash.conf` - Log processing pipeline
- `elasticsearch-index-template.json` - Index mappings
- `KIBANA_SETUP.md` - Dashboard setup guide

## Configuration

### FiveM Resource

Edit `anticheat-resource/shared/config.lua` to customize:

- Detection thresholds
- Enabled modules
- Sanction settings
- API connectivity

### Web Admin

Environment variables:

- `PORT` - Server port (default: 3000)
- `DB_PATH` - SQLite database path
- `JWT_SECRET` - JWT signing secret
- `LOG_DIR` - Log directory

## API Endpoints

| Endpoint                         | Description                |
| -------------------------------- | -------------------------- |
| `POST /api/auth/login`           | Admin login                |
| `GET /api/players`               | List players               |
| `GET /api/detections`            | Detection logs             |
| `GET /api/sanctions`             | Sanction list              |
| `GET /api/rulesets`              | Get rulesets               |
| `GET /api/fivem/rulesets/active` | Get active ruleset (FiveM) |
| `POST /api/fivem/detections`     | Report detection (FiveM)   |

## Project Structure

```
├── anticheat-resource/     # FiveM Lua resource
│   ├── fxmanifest.lua
│   ├── shared/config.lua
│   ├── server/
│   │   ├── main.lua
│   │   ├── api.lua
│   │   ├── logger.lua
│   │   └── detections/
│   └── client/main.lua
│
├── anticheat-admin/        # Node.js backend + React frontend
│   ├── src/
│   │   ├── server.js
│   │   ├── db/
│   │   ├── routes/
│   │   └── middleware/
│   └── frontend/
│       └── src/
│
└── elk-config/             # ELK Stack configuration
    ├── filebeat.yml
    ├── logstash.conf
    └── elasticsearch-index-template.json
```

## License

MIT License - See LICENSE file for details.
