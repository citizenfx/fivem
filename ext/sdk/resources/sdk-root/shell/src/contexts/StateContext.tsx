import React from 'react';
import { AppStates } from 'shared/api.types';
import { stateApi } from 'shared/api.events';
import { useApiMessage, useOpenFlag } from 'utils/hooks';
import { logger } from 'utils/logger';


const log = logger('StateContext');

export interface StateContext {
  state: AppStates,
  gameLaunched: boolean,

  toolbarOpen: boolean,
  openToolbar: () => void,
  closeToolbar: () => void,
};

const defaultState: StateContext = {
  state: AppStates.booting,
  gameLaunched: false,

  toolbarOpen: true,
  openToolbar: () => {},
  closeToolbar: () => {},
};

export const StateContext = React.createContext<StateContext>(defaultState);

export const StateContextProvider = React.memo(function StateContextProvider({ children }) {
  const [state, setState] = React.useState<AppStates>(defaultState.state);
  const [gameLaunched, setGameLaunched] = React.useState(defaultState.gameLaunched);
  const [toolbarOpen, openToolbar, closeToolbar] = useOpenFlag(defaultState.toolbarOpen);

  useApiMessage(stateApi.state, (newState: AppStates) => {
    setState(newState);
  }, [setState]);

  useApiMessage(stateApi.gameLaunched, () => {
    log('Game launched!');
    setGameLaunched(true);
  }, [setGameLaunched]);

  const value = {
    state,
    gameLaunched,

    toolbarOpen,
    openToolbar,
    closeToolbar,
  };

  return (
    <StateContext.Provider value={value}>
      {children}
    </StateContext.Provider>
  );
});
