import { GameContext } from 'contexts/GameContext';
import { ServerContext } from 'contexts/ServerContext';
import { TheiaContext } from 'contexts/TheiaContext';
import { ServerStates } from 'shared/api.types';
import { useSdkMessage } from 'utils/hooks';
import { sendCommand } from 'utils/sendCommand';
import * as React from 'react';

export const GameConnectionManager = React.memo(function GameConnectionManager() {
  const { serverState, clientConnected, setClientConnected } = React.useContext(ServerContext);
  const { sendTheiaMessage } = React.useContext(TheiaContext);
  const { gameLaunched } = React.useContext(GameContext);

  const pendingClientConnect = React.useRef(false);

  React.useEffect(() => {
    if (clientConnected && !gameLaunched) {
      return setClientConnected(false);
    }

    switch (serverState) {
      case ServerStates.up: {
        const shouldIssueConnectCommand = !clientConnected
          && gameLaunched
          && !pendingClientConnect.current;

        if (shouldIssueConnectCommand) {
          pendingClientConnect.current = true;

          sendCommand('connect 127.0.0.1:30120');

          sendTheiaMessage({
            type: 'fxdk:openGameView',
          });
        }

        break;
      }

      case ServerStates.down: {
        if (gameLaunched && (clientConnected || pendingClientConnect.current)) {
          pendingClientConnect.current = false;
          window.setFPSLimit(60);
          sendCommand('disconnect');
          setClientConnected(false);
        }

        break;
      }
    }
  }, [serverState, gameLaunched, clientConnected, setClientConnected, sendTheiaMessage]);

  useSdkMessage('connected', () => {
    pendingClientConnect.current = false;
    window.setFPSLimit(0);
    setClientConnected(true);

    console.log('Client connected!');
  }, [setClientConnected]);

  return null;
});
