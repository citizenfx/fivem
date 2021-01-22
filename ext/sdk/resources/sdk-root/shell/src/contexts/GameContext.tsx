import * as React from 'react';
import { gameApi } from 'shared/api.events';
import { NetLibraryConnectionState, SDKGameProcessState } from 'shared/native.enums';
import { sendApiMessage } from 'utils/api';
import { useApiMessage } from 'utils/hooks';

export interface GameContext {
  gameLaunched: boolean,
  gameProcessState: SDKGameProcessState,
  connectionState: NetLibraryConnectionState,

  startGame: () => void,
  stopGame: () => void,
  restartGame: () => void,
}
export const GameContext = React.createContext<GameContext>({
  gameLaunched: false,
  gameProcessState: SDKGameProcessState.GP_STOPPED,
  connectionState: NetLibraryConnectionState.CS_IDLE,

  startGame: () => {},
  stopGame: () => {},
  restartGame: () => {},
});

export const GameContextProvider = React.memo(function GameContextProvider({ children }) {
  const [gameLaunched, setGameLaunched] = React.useState(false);
  const [gameProcessState, setGameProcessState] = React.useState(SDKGameProcessState.GP_STOPPED);
  const [connectionState, setConnectionState] = React.useState(NetLibraryConnectionState.CS_IDLE);

  const startGame = React.useCallback(() => sendApiMessage(gameApi.start), []);
  const stopGame = React.useCallback(() => sendApiMessage(gameApi.stop), []);
  const restartGame = React.useCallback(() => sendApiMessage(gameApi.restart), []);

  useApiMessage(gameApi.gameLaunched, (launched) => {
    setGameLaunched(launched);
  }, [setGameLaunched]);

  useApiMessage(gameApi.gameProcessStateChanged, ({ current }) => {
    setGameProcessState(current);
  }, [setGameProcessState]);

  useApiMessage(gameApi.connectionStateChanged, ({ current }) => {
    setConnectionState(current);
  }, [setConnectionState]);

  React.useEffect(() => {
    sendApiMessage(gameApi.ack);
  }, []);

  useApiMessage(gameApi.ack, (data) => {
    setGameLaunched(data.gameLaunched);
    setGameProcessState(data.gameProcessState);
    setConnectionState(data.connectionState);
  }, [setGameLaunched, setGameProcessState, setConnectionState]);

  const value: GameContext = {
    gameLaunched,
    gameProcessState,
    connectionState,

    startGame,
    stopGame,
    restartGame,
  };

  return (
    <GameContext.Provider value={value}>
      {children}
    </GameContext.Provider>
  );
});
