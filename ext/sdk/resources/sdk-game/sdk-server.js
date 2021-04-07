function joaat(key) {
  key = key.toLowerCase();

  const hash = new Uint32Array(1);

  for (const i in key) {
    hash[0] += key.charCodeAt(i);
    hash[0] += hash[0] << 10;
    hash[0] ^= hash[0] >>> 6;
  }

  hash[0] += hash[0] << 3;
  hash[0] ^= hash[0] >>> 11;
  hash[0] += hash[0] << 15;

  return '0x' + hash[0].toString(16).toUpperCase();
}

if (GetConvarInt('sv_fxdkServerMode', 0) === 0) {
  const net = require('net');
  const byline = require('./byline');

  const REGISTER_CONSOLE_LISTENER = joaat('REGISTER_CONSOLE_LISTENER');
  const GET_CONSOLE_BUFFER = joaat('GET_CONSOLE_BUFFER');

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

  const lineStream = byline.createStream();

  lineStream.on('data', (msg) => {
    try {
      const [type, data] = JSON.parse(msg.toString());

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

  setTimeout(() => {
    send('ready');

    send('consoleBuffer', Citizen.invokeNative(GET_CONSOLE_BUFFER, Citizen.resultAsString()));
    Citizen.invokeNative(REGISTER_CONSOLE_LISTENER, Citizen.makeRefFunction((channel, message) => send('console', { channel, message })));

    // Check for resources state that we can't catch with `onResourceStart` as they start before sdk-game
    ['sessionmanager', 'sessionmanager-rdr3'].map((resourceName) => {
      const resourceState = GetResourceState(resourceName);

      if (resourceState === 'starting' || resourceState === 'started') {
        resourcesState[resourceName] = true;
        sendState();
      }
    });
  }, 0);
} else {
  console.log('^3Hooray, FxDK server mode!');
}
