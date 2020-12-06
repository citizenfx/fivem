import React from 'react';
import { StateContext } from 'contexts/StateContext';
import { ProjectContext } from 'contexts/ProjectContext';
import { AppStates } from 'shared/api.types';
import { sendApiMessage } from 'utils/api';
import { TheiaPersonality } from 'personalities/Theia';
import { Toolbar } from './Toolbar/Toolbar';
import { Welcome } from './Welcome/Welcome';
import { Update } from './Updater/Updater';
import s from './Shell.module.scss';


export function Shell() {
  const { state } = React.useContext(StateContext);
  const { project, recentProjects } = React.useContext(ProjectContext);

  const showWelcome = state === AppStates.ready && !project && recentProjects.length === 0;
  const showUpdate = state === AppStates.preparing;

  React.useEffect(() => {
    sendApiMessage('ackState');
  }, []);

  return (
    <div className={s.root}>
      <Toolbar />

      {showUpdate && (
        <Update />
      )}

      {showWelcome && (
        <Welcome />
      )}

      <TheiaPersonality />
    </div>
  );
}
