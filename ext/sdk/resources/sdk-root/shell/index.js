global.nativeRequire = require;

// This isn't an error, top-level script require is based off of resource root
require('./shell/build_server');
