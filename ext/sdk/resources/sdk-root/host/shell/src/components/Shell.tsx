import React from 'react';
import { Toolbar } from './Toolbar/Toolbar';
import { StateContext } from '../contexts/StateContext';
import { sendApiMessage } from '../utils/api';
import { TheiaPersonality } from '../personalities/Theia';
import { ProjectContext } from '../contexts/ProjectContext';
import { Welcome } from './Welcome/Welcome';
import { States } from '../sdkApi/api.types';
import s from './Shell.module.scss';


export function Shell() {
  const { state } = React.useContext(StateContext);
  const { project, recentProjects } = React.useContext(ProjectContext);

  const showWelcome = state === States.ready && !project && recentProjects.length === 0;

  React.useEffect(() => {
    sendApiMessage('ackState');
  }, []);

  return (
    <div className={s.root}>
      <Toolbar />

      {showWelcome && (
        <Welcome />
      )}

      <TheiaPersonality />
    </div>
  );
}
