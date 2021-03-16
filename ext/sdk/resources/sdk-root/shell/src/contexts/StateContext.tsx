import React from 'react';
import { AppStates } from 'shared/api.types';
import { stateApi } from 'shared/api.events';
import { useApiMessage, useOpenFlag } from 'utils/hooks';
import { hasNewChangelogEntries } from 'components/Changelog/Changelog.utils';


export const clampToolbarWidth = (width: number): number => {
  return Math.max(250, Math.min(document.body.offsetWidth / 2, width));
};

const initialToolbarWidth = clampToolbarWidth(parseInt(localStorage.toolbarWidth, 0) || 430);

export interface StateContext {
  state: AppStates,

  toolbarWidth: number,
  setToolbarWidth: (width: number) => void,

  updaterOpen: boolean,
  openUpdater: () => void,
  closeUpdater: () => void,

  toolbarOpen: boolean,
  openToolbar: () => void,
  closeToolbar: () => void,

  changelogOpen: boolean,
  openChangelog: () => void,
  closeChangelog: () => void,
};

const defaultState: StateContext = {
  state: AppStates.booting,

  toolbarWidth: 0,
  setToolbarWidth: () => {},

  updaterOpen: hasNewChangelogEntries(),
  openUpdater: () => {},
  closeUpdater: () => {},

  toolbarOpen: true,
  openToolbar: () => {},
  closeToolbar: () => {},

  changelogOpen: false,
  openChangelog: () => {},
  closeChangelog: () => {},
};

export const StateContext = React.createContext<StateContext>(defaultState);

export const StateContextProvider = React.memo(function StateContextProvider({ children }) {
  const [state, setState] = React.useState<AppStates>(defaultState.state);

  const [updaterOpen, openUpdater, closeUpdater] = useOpenFlag(defaultState.updaterOpen);
  const [toolbarOpen, openToolbar, closeToolbar] = useOpenFlag(defaultState.toolbarOpen);
  const [changelogOpen, openChangelog, closeChangelog] = useOpenFlag(defaultState.changelogOpen);

  const [toolbarWidth, saveToolbarWidth] = React.useState<number>(initialToolbarWidth);
  const setToolbarWidth = React.useCallback((width: number) => {
    const newWidth = clampToolbarWidth(width);

    saveToolbarWidth(newWidth);

    localStorage.toolbarWidth = newWidth;
  }, []);

  useApiMessage(stateApi.state, (newState: AppStates) => {
    setState(newState);
  }, [setState]);

  const value = {
    state,

    toolbarWidth,
    setToolbarWidth,

    updaterOpen,
    openUpdater,
    closeUpdater,

    toolbarOpen,
    openToolbar,
    closeToolbar,

    changelogOpen,
    openChangelog,
    closeChangelog,
  };

  return (
    <StateContext.Provider value={value}>
      {children}
    </StateContext.Provider>
  );
});
