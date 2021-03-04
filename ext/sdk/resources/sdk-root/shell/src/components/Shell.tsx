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
import { useTask } from 'contexts/TaskContext';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';


export function Shell() {
  const { state, updaterOpen, changelogOpen } = React.useContext(StateContext);
  const { project, creatorOpen, openerOpen } = React.useContext(ProjectContext);

  const projectCreatingTask = useTask(projectCreatingTaskName);
  const projectLoadingTask = useTask(projectLoadingTaskName);

  const showToolbar = Boolean(
    project || creatorOpen || openerOpen || projectCreatingTask || projectLoadingTask
  );
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
