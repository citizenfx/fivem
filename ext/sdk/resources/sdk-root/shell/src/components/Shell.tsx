import React from 'react';
import { StateContext } from 'contexts/StateContext';
import { ProjectContext } from 'contexts/ProjectContext';
import { AppStates } from 'shared/api.types';
import { sendApiMessage } from 'utils/api';
import { TheiaPersonality } from 'personalities/Theia';
import { Toolbar } from './Toolbar/Toolbar';
import { Welcome } from './Welcome/Welcome';
import { Updater } from './Updater/Updater';
import { ChangelogModal } from './Changelog/Changelog.modal';
import s from './Shell.module.scss';


export function Shell() {
  const { state, updaterOpen, changelogOpen } = React.useContext(StateContext);
  const { project, creatorOpen, openerOpen } = React.useContext(ProjectContext);

  const showToolbar = !!project || creatorOpen || openerOpen;
  const showWelcome = state === AppStates.ready && !showToolbar;
  const showUpdater = state === AppStates.preparing || updaterOpen;

  React.useEffect(() => {
    sendApiMessage('ackState');
  }, []);

  return (
    <div className={s.root}>
      {!showWelcome && (
        <Toolbar />
      )}

      {showUpdater && (
        <Updater />
      )}

      {showWelcome && (
        <Welcome />
      )}

      {changelogOpen && (
        <ChangelogModal />
      )}

      <TheiaPersonality />
    </div>
  );
}
