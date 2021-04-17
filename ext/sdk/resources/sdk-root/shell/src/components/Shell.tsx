import React from 'react';
import { observer } from 'mobx-react-lite';
import { AppStates } from 'shared/api.types';
import { Toolbar } from './Toolbar/Toolbar';
import { Welcome } from './Welcome/Welcome';
import { Updater } from './Updater/Updater';
import { ChangelogModal } from './Changelog/Changelog.modal';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';
import { TheiaPersonality } from 'personalities/TheiaPersonality/TheiaPersonality';
import { ShellState } from 'store/ShellState';
import { GameState } from 'store/GameState';
import { StatusState } from 'store/StatusState';
import { TaskState } from 'store/TaskState';
import { ServerState } from 'store/ServerState';
import { ProjectState } from 'store/ProjectState';
import s from './Shell.module.scss';
import { WorldEditorPersonality } from 'personalities/WorldEditorPersonality/WorldEditorPersonality';
import { NotificationState } from 'store/NotificationState';


export const Shell = observer(function Shell() {
  const projectCreatingTask = TaskState.get(projectCreatingTaskName);
  const projectLoadingTask = TaskState.get(projectLoadingTaskName);

  const showToolbar = Boolean(
    ProjectState.hasProject || ProjectState.creatorOpen || ProjectState.openerOpen || projectCreatingTask || projectLoadingTask
  );
  const showWelcome = ShellState.appState === AppStates.ready && !showToolbar;
  const showUpdater = ShellState.appState === AppStates.preparing || ShellState.updaterOpen;

  React.useEffect(() => {
    ShellState.ack();
    GameState.ack();
    StatusState.ack();
    TaskState.ack();
    ServerState.ack();
    NotificationState.ack();
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

      {ShellState.changelogOpen && (
        <ChangelogModal />
      )}

      <TheiaPersonality />

      {ShellState.isWorldEditor && (
        <WorldEditorPersonality />
      )}
    </div>
  );
});
