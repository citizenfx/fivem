import React from 'react';
import { DownloadState, States } from 'shared/api.types';
import { stateApi } from 'shared/api.events';
import { useApiMessage, useOpenFlag } from 'utils/hooks';


export interface StateContext {
  state: States,
  gameLaunched: boolean,

  toolbarOpen: boolean,
  openToolbar: () => void,
  closeToolbar: () => void,

  fxserverDownload: DownloadState,
  fxserverUnpack: DownloadState,
};

const defaultState: StateContext = {
  state: States.booting,
  gameLaunched: false,

  toolbarOpen: true,
  openToolbar: () => {},
  closeToolbar: () => {},

  fxserverDownload: {
    total: 0,
    downloaded: 0,
  },
  fxserverUnpack: {
    total: 0,
    downloaded: 0,
  },
};

export const StateContext = React.createContext<StateContext>(defaultState);

export const StateContextProvider = React.memo(function StateContextProvider({ children }) {
  const [state, setState] = React.useState<States>(defaultState.state);
  const [gameLaunched, setGameLaunched] = React.useState(defaultState.gameLaunched);
  const [toolbarOpen, openToolbar, closeToolbar] = useOpenFlag(defaultState.toolbarOpen);

  const [fxserverDownload, setFxserverDownload] = React.useState(defaultState.fxserverDownload);
  const [fxserverUnpack, setFxserverUnpack] = React.useState(defaultState.fxserverUnpack);

  useApiMessage(stateApi.state, (newState: States) => {
    setState(newState);
  }, [setState]);

  useApiMessage(stateApi.fxserverDownload, (data) => {
    setFxserverDownload(data);
  }, [setFxserverDownload]);

  useApiMessage(stateApi.fxserverUnpack, (data) => {
    setFxserverUnpack(data);
  }, [setFxserverUnpack]);

  useApiMessage(stateApi.gameLaunched, () => {
    setGameLaunched(true);
  }, [setGameLaunched]);

  const value = {
    state,
    gameLaunched,

    toolbarOpen,
    openToolbar,
    closeToolbar,

    fxserverDownload,
    fxserverUnpack,
  };

  return (
    <StateContext.Provider value={value}>
      {children}
    </StateContext.Provider>
  );
});
