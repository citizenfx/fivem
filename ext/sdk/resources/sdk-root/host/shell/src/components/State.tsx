import React from 'react';

import { serverEvents } from '../utils/serverEvents';


export enum States {
  booting = 0,
  preparing,
  ready,
};

export interface TDownloadState {
  total: number,
  downloaded: number,
};

export interface TStateContext {
  state: States,
  fxserverDownload: TDownloadState,
  fxserverUnpack: TDownloadState,
};

const defaultState: TStateContext = {
  state: States.booting,
  fxserverDownload: {
    total: 0,
    downloaded: 0,
  },
  fxserverUnpack: {
    total: 0,
    downloaded: 0,
  },
};

export const StateContext = React.createContext<TStateContext>(defaultState);

export const StateProvider = React.memo(({ children }) => {
  const setStateRef = React.useRef<any>(null);
  const [state, setState] = React.useState(defaultState);
  setStateRef.current = (newState) => setState({ ...state, ...newState });

  React.useEffect(() => {
    const handler = (e: MessageEvent) => {
      const event = JSON.parse(e.data);

      switch (event.type) {
        case 'state:transition': {
          return setStateRef.current({ state: States[event.data] });
        }

        case 'state:fxserverDownload': {
          return setStateRef.current({ fxserverDownload: event.data });
        }

        case 'state:fxserverUnpack': {
          return setStateRef.current({ fxserverUnpack: event.data });
        }
      }
    };

    serverEvents.addEventListener('message', handler);

    return () => serverEvents.removeEventListener('message', handler);
  }, []);

  return (
    <StateContext.Provider value={state}>
      {children}
    </StateContext.Provider>
  );
});
