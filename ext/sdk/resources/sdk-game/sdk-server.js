const net = require('net');
const byline = require('./byline');

const pipe = '\\\\.\\pipe\\fxdk_fxserver_sdk_game';
const ipc = net.createConnection(pipe);
const send = (type, data) => {
    ipc.write(JSON.stringify([type, data]) + '\n');
}

const sendState = () => {
    send('state', resourcesState);
};

const resourcesState = {};

on('onResourceStart', (name) => {
    resourcesState[name] = true;
    sendState();
});

on('onResourceStop', (name) => {
    resourcesState[name] = false;
    sendState();
});

send('ready');

const lineStream = byline.createStream();

lineStream.on('data', (msg) => {
    try {
        const [type, data] = JSON.parse(msg.toString());

        console.log('ipcMessage received', msg.toString(), type, data);

        switch (type) {
            case 'state': {
                return sendState();
            }
            case 'restart': {
                if (resourcesState[data]) {
                    ExecuteCommand(`restart ${data}`);
                }
                return;
            }
            case 'stop': {
                if (resourcesState[data]) {
                    ExecuteCommand(`stop ${data}`);
                }
                return;
            }
            case 'start': {
                if (!resourcesState[data]) {
                    ExecuteCommand(`start ${data}`);
                }
                return;
            }
            case 'refresh': {
                return ExecuteCommand('refresh');
            }
        }
    } catch (e) {
        console.log('Invalid data received', msg.toString(), e);
    }
});

ipc.pipe(lineStream);
