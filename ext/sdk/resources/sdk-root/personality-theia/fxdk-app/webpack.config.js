const path = require('path');
const frontendConfig = require('./gen-webpack.config.js');

frontendConfig.entry = path.join(__dirname, 'frontend.js');
frontendConfig.performance = false;

module.exports = frontendConfig;
