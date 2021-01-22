import { GameContext } from 'contexts/GameContext';
import { ServerContext } from 'contexts/ServerContext';
import { TheiaContext } from 'contexts/TheiaContext';
import { ServerStates } from 'shared/api.types';
import { useSdkMessage } from 'utils/hooks';
import { sendCommand } from 'utils/sendCommand';
import * as React from 'react';
import { logger } from 'utils/logger';
import { NetLibraryConnectionState } from 'shared/native.enums';

const log = logger('GameConnectionManager');
const RECONNECT_INTERVAL = 1000;

export const GameConnectionManager = React.memo(function GameConnectionManager() {
  const { serverState, clientConnected, setClientConnected } = React.useContext(ServerContext);
  const { sendTheiaMessage } = React.useContext(TheiaContext);
  const { gameLaunched, connectionState } = React.useContext(GameContext);

  const pendingClientConnect = React.useRef(false);
  const pendingClientReconnectTimer = React.useRef(null);

  // Clear reconnect interval if connection state transitioned from idle
  React.useEffect(() => {
    if (pendingClientReconnectTimer.current !== null) {
      if (connectionState !== NetLibraryConnectionState.CS_IDLE) {
        clearInterval(pendingClientReconnectTimer.current);
        pendingClientReconnectTimer.current = null;
      }
    }
  }, [connectionState]);

  // Cleanup on unmount
  React.useEffect(() => () => {
    if (pendingClientReconnectTimer.current !== null) {
      clearInterval(pendingClientReconnectTimer.current);
    }
  }, []);

  React.useEffect(() => {
    log({
      serverState: ServerStates[serverState],
      clientConnected,
      gameLaunched,
      connectionState: NetLibraryConnectionState[connectionState],
      pendingClientConnect: pendingClientConnect.current,
    });

    if (clientConnected && !gameLaunched) {
      return setClientConnected(false);
    }

    switch (serverState) {
      case ServerStates.up: {
        const shouldIssueConnectCommand = !clientConnected
          && gameLaunched
          && !pendingClientConnect.current;

        if (shouldIssueConnectCommand) {
          log('Issuing connect command to game');

          pendingClientConnect.current = true;

          sendCommand('connect 127.0.0.1:30120');

          sendTheiaMessage({
            type: 'fxdk:openGameView',
          });

          pendingClientReconnectTimer.current = setInterval(() => {
            sendCommand('connect 127.0.0.1:30120');
          }, RECONNECT_INTERVAL);
        }

        break;
      }

      case ServerStates.down: {
        if (gameLaunched && (clientConnected || pendingClientConnect.current)) {
          log('Disconnecting game from server');

          pendingClientConnect.current = false;
          window.setFPSLimit(60);
          sendCommand('disconnect');
          setClientConnected(false);
        }

        break;
      }
    }
  }, [serverState, gameLaunched, clientConnected, connectionState, setClientConnected, sendTheiaMessage]);

  useSdkMessage('connected', () => {
    pendingClientConnect.current = false;
    window.setFPSLimit(0);
    setClientConnected(true);

    console.log('Client connected!');
  }, [setClientConnected]);

  return null;
});
