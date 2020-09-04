// @ts-check
const { BackendApplicationConfigProvider } = require('@theia/core/lib/node/backend-application-config-provider');
const main = require('@theia/core/lib/node/main');
BackendApplicationConfigProvider.set({});

const serverModule = require('./server');
const serverAddress = main.start(serverModule());
serverAddress.then(function ({ port, address }) {
    if (process && process.send) {
        process.send({ port, address });
    }
});
module.exports = serverAddress;
