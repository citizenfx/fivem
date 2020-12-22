import React from 'react';
import { AppStates } from 'shared/api.types';
import { stateApi } from 'shared/api.events';
import { useApiMessage, useOpenFlag } from 'utils/hooks';
import { logger } from 'utils/logger';
import { hasNewChangelogEntries } from 'components/Changelog/Changelog.utils';


const log = logger('StateContext');

export interface StateContext {
  state: AppStates,
  gameLaunched: boolean,

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
  gameLaunched: false,

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
  const [gameLaunched, setGameLaunched] = React.useState(defaultState.gameLaunched);

  const [updaterOpen, openUpdater, closeUpdater] = useOpenFlag(defaultState.updaterOpen);
  const [toolbarOpen, openToolbar, closeToolbar] = useOpenFlag(defaultState.toolbarOpen);
  const [changelogOpen, openChangelog, closeChangelog] = useOpenFlag(defaultState.changelogOpen);

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
