import React from 'react';
import { States } from 'shared/api.types';
import { stateApi } from 'shared/api.events';
import { useApiMessage, useOpenFlag } from 'utils/hooks';


export interface StateContext {
  state: States,
  gameLaunched: boolean,

  toolbarOpen: boolean,
  openToolbar: () => void,
  closeToolbar: () => void,
};

const defaultState: StateContext = {
  state: States.booting,
  gameLaunched: false,

  toolbarOpen: true,
  openToolbar: () => {},
  closeToolbar: () => {},
};

export const StateContext = React.createContext<StateContext>(defaultState);

export const StateContextProvider = React.memo(function StateContextProvider({ children }) {
  const [state, setState] = React.useState<States>(defaultState.state);
  const [gameLaunched, setGameLaunched] = React.useState(defaultState.gameLaunched);
  const [toolbarOpen, openToolbar, closeToolbar] = useOpenFlag(defaultState.toolbarOpen);

  useApiMessage(stateApi.state, (newState: States) => {
    setState(newState);
  }, [setState]);

  useApiMessage(stateApi.gameLaunched, () => {
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
