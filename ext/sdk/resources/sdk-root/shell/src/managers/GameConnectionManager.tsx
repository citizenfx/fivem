import * as React from 'react';
import { ServerStates } from 'shared/api.types';
import { useWindowEvent } from 'utils/hooks';
import { sendCommandToGameClient } from 'utils/sendCommand';
import { logger } from 'utils/logger';
import { NetLibraryConnectionState } from 'shared/native.enums';
import { observer } from 'mobx-react-lite';
import { GameState } from 'store/GameState';
import { ServerState } from 'store/ServerState';
import { TheiaState } from 'personalities/TheiaPersonality/TheiaState';

const log = logger('GameConnectionManager');
const RECONNECT_INTERVAL = 1000;

export const GameConnectionManager = observer(function GameConnectionManager() {
  const pendingClientConnect = React.useRef(false);
  const pendingClientReconnectTimer = React.useRef(null);

  // Clear reconnect interval if connection state transitioned from idle
  React.useEffect(() => {
    if (pendingClientReconnectTimer.current !== null) {
      if (GameState.connectionState !== NetLibraryConnectionState.CS_IDLE) {
        clearInterval(pendingClientReconnectTimer.current);
        pendingClientReconnectTimer.current = null;
      }
    }
  }, [GameState.connectionState]);

  // Cleanup on unmount
  React.useEffect(() => () => {
    if (pendingClientReconnectTimer.current !== null) {
      clearInterval(pendingClientReconnectTimer.current);
    }
  }, []);

  React.useEffect(() => {
    log({
      serverState: ServerStates[ServerState.state],
      clientConnected: ServerState.clientConnected,
      gameLaunched: GameState.launched,
      connectionState: NetLibraryConnectionState[GameState.connectionState],
      pendingClientConnect: pendingClientConnect.current,
    });

    if (ServerState.clientConnected && !GameState.launched) {
      return ServerState.setClientConnected(false);
    }

    switch (true) {
      case ServerState.isUp: {
        const shouldIssueConnectCommand = !ServerState.clientConnected
          && GameState.launched
          && !pendingClientConnect.current;

        if (shouldIssueConnectCommand) {
          log('Issuing connect command to game');

          pendingClientConnect.current = true;

          sendCommandToGameClient('connect 127.0.0.1:30120');

          TheiaState.openGameView();

          pendingClientReconnectTimer.current = setInterval(() => {
            sendCommandToGameClient('connect 127.0.0.1:30120');
          }, RECONNECT_INTERVAL);
        }

        break;
      }

      case ServerState.isDown: {
        if (GameState.launched && (ServerState.clientConnected || pendingClientConnect.current)) {
          log('Disconnecting game from server');

          pendingClientConnect.current = false;
          window.setFPSLimit(60);
          sendCommandToGameClient('disconnect');
          ServerState.setClientConnected(false);
        }

        break;
      }
    }
  }, [GameState.launched, ServerState.state, ServerState.clientConnected, GameState.connectionState]);

  useWindowEvent('connected', () => {
    pendingClientConnect.current = false;
    window.setFPSLimit(0);
    ServerState.setClientConnected(true);

    console.log('Client connected!');
  }, []);

  return null;
});
