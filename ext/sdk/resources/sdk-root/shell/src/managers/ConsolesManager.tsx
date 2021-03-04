import { GameContext } from 'contexts/GameContext';
import { TheiaContext } from 'contexts/TheiaContext';
import * as React from 'react';
import { serverApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage, useSdkMessage } from 'utils/hooks';

export const ConsolesManager = React.memo(function ConsolesManager() {
  const { sendTheiaMessage } = React.useContext(TheiaContext);
  const { gameLaunched } = React.useContext(GameContext);

  // Server console
  useApiMessage(serverApi.bufferedOutput, (data: string) => {
    sendTheiaMessage({ type: 'fxdk:serverOutput', data });
  }, [sendTheiaMessage]);

  useApiMessage(serverApi.structuredOutputMessage, (data) => {
    sendTheiaMessage({ type: 'fxdk:serverOutputStructured', data });
  }, [sendTheiaMessage]);

  useApiMessage(serverApi.clearOutput, () => {
    sendTheiaMessage({ type: 'fxdk:clearServerOutput' });
  }, [sendTheiaMessage]);

  useSdkMessage('server:sendCommand', (command) => {
    sendApiMessage(serverApi.sendCommand, command);
  });

  // Game console
  useSdkMessage('game:consoleMessage', (data: { channel: string, message: string }) => {
    if (!data.message.trim()) {
      return;
    }

    sendTheiaMessage({ type: 'fxdk:gameStructuredMessage', data });
  }, [sendTheiaMessage]);

  React.useEffect(() => {
    if (!gameLaunched) {
      sendTheiaMessage({ type: 'fxdk:clearGameOutput' });
    }
  }, [gameLaunched, sendTheiaMessage]);

  return null;
});
