import '@citizenfx/server';

let currentHosting: string | void;
let hostsPendingRelease: string[] = [];

function sendResult(host: string, result: 'free' | 'wait' | 'go' | 'conflict') {
  emitNet('sessionHostResult', host, result);
}

function releaseHosts() {
  for (const host of hostsPendingRelease) {
    sendResult(host, 'free');
  }
}

onNet('hostingSession', () => {
  if (currentHosting) {
    sendResult(source, 'wait');

    return hostsPendingRelease.push(source);
  }

  if (GetHostId() && (GetPlayerLastMsg(GetHostId()) < 1000)) {
    return sendResult(source, 'conflict');
  }

  hostsPendingRelease = [];
  currentHosting = source;

  sendResult(source, 'go');

  setTimeout(() => {
    if (currentHosting !== undefined) {
      currentHosting = undefined;

      releaseHosts();
    }
  }, 5000);
});

onNet('hostedSession', () => {
  if (currentHosting !== source) {
    console.log(`${currentHosting} ~= ${source}`);
    return;
  }

  releaseHosts();
  currentHosting = undefined;
});

EnableEnhancedHostSupport(true);
