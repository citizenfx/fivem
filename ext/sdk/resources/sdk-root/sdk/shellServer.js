const cors = require('cors');
const express = require('express');


const shellPort = 35419;
const shellApp = express();

const shellEndpointHeaders = {
  'Access-Control-Allow-Origin': '*',
};

const shellEventsHeaders = {
  ...shellEndpointHeaders,
  'Content-Type': 'text/event-stream',
  'Connection': 'keep-alive',
  'Cache-Control': 'no-cache',
};

let clients = {};

const sendClientEvent = (event) => {
  Object.values(clients).forEach((res) => res.write('data:' + JSON.stringify(event) + '\n\n'));
};

const start = (shellPath) => new Promise((resolve) => {
  let resolved = false;

  shellApp.get('/ready', (req, res) => {
    res.writeHead(200, shellEndpointHeaders);
    res.end();

    if (!resolved) {
      resolved = true;

      resolve();
    } else {
      sendClientEvent({ type: 'state:transition', data: 'ready' });
    }
  })

  shellApp.get('/events', (req, res, _) => {
    res.writeHead(200, shellEventsHeaders);

    const clientId = Math.random().toString(16);
    clients[clientId] = res;

    res.on('close', () => {
      delete clients[clientId];
    });
  });

  shellApp.use(cors({ origin: '*' }));
  shellApp.use(express.static(shellPath));

  shellApp.listen(shellPort, () => {
    console.log(`Shell server started at http://localhost:${shellPort}/`);
  });
});

module.exports = {
  start,
  sendClientEvent,
};
