# Anticheat Admin Panel

Web-based administration panel for FiveM/RedM anticheat system with real-time monitoring, player management, and ruleset configuration.

## Features

- 🛡️ Real-time detection monitoring via WebSocket
- 👥 Player management with trust levels
- ⚖️ Sanction management (warn, kick, ban)
- 📋 Configurable rulesets with detection types
- 📊 Dashboard with statistics
- 🔐 Role-based access control (moderator, admin, superadmin)
- 📝 Audit logging

## Quick Start

### Prerequisites

- Node.js 18+
- npm or yarn
- (Optional) MySQL 5.7+ or MariaDB 10.2+ for production

### Installation

```bash
# Install backend dependencies
cd anticheat-admin
npm install

# Install frontend dependencies
cd frontend
npm install
cd ..
```

### Database Setup

The system supports two database backends:

#### SQLite (Default - Development)

No additional setup required. The database file will be created automatically at `data/anticheat.db`.

```bash
# Initialize database with default data
npm run init-db
```

#### MySQL (Recommended for Production)

1. Create a MySQL database:

```sql
CREATE DATABASE anticheat_admin CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER 'anticheat'@'localhost' IDENTIFIED BY 'your_secure_password';
GRANT ALL PRIVILEGES ON anticheat_admin.* TO 'anticheat'@'localhost';
FLUSH PRIVILEGES;
```

2. Set environment variables:

```bash
export DB_TYPE=mysql
export MYSQL_HOST=localhost
export MYSQL_PORT=3306
export MYSQL_USER=anticheat
export MYSQL_PASSWORD=your_secure_password
export MYSQL_DATABASE=anticheat_admin
export MYSQL_POOL_SIZE=10  # Optional, default: 10
```

3. Initialize the database:

```bash
npm run init-db
```

### Running the Server

```bash
# Development mode (with auto-reload)
npm run dev

# Production mode
npm start
```

The server will start on `http://localhost:3000` by default.

### Environment Variables

| Variable | Default | Description |
|----------|---------|-------------|
| `PORT` | `3000` | Server port |
| `NODE_ENV` | `development` | Environment mode |
| `DB_TYPE` | `sqlite` | Database type: `sqlite` or `mysql` |
| `DB_PATH` | `./data/anticheat.db` | SQLite database path |
| `MYSQL_HOST` | `localhost` | MySQL host |
| `MYSQL_PORT` | `3306` | MySQL port |
| `MYSQL_USER` | `anticheat` | MySQL username |
| `MYSQL_PASSWORD` | - | MySQL password |
| `MYSQL_DATABASE` | `anticheat_admin` | MySQL database name |
| `MYSQL_POOL_SIZE` | `10` | MySQL connection pool size |
| `JWT_SECRET` | (random) | JWT signing secret |
| `JWT_EXPIRY` | `24h` | JWT token expiry |
| `ADMIN_PASSWORD` | `changeme123` | Default admin password |
| `CORS_ORIGIN` | `*` | CORS allowed origins |

## API Endpoints

| Endpoint | Description |
|----------|-------------|
| `/api/auth` | Authentication (login, logout, me) |
| `/api/players` | Player management |
| `/api/detections` | Detection logs |
| `/api/sanctions` | Ban/kick management |
| `/api/rulesets` | Ruleset configuration |
| `/api/stats` | Dashboard statistics |
| `/api/fivem` | FiveM server integration |
| `/ws` | WebSocket for real-time updates |

## FiveM Integration

Add the API key to your FiveM server's `server.cfg`:

```cfg
set anticheat_api_key "your-api-key-here"
set anticheat_api_url "http://your-admin-server:3000/api/fivem"
```

## Rate Limiting

The API includes rate limiting to prevent abuse:

- **Login**: 5 attempts per 15 minutes (30-minute block after exceeding)
- **API**: 100 requests per minute
- **FiveM**: 50 requests per second

## Security Notes

1. **Change the default admin password** after first login
2. Set a strong `JWT_SECRET` in production
3. Use HTTPS in production (reverse proxy recommended)
4. Restrict `CORS_ORIGIN` to your frontend domain
5. Keep the FiveM API key secure

## License

Part of the Cfx.re FiveM project.

