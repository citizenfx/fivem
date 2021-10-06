import React from 'react';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ProjectState } from 'store/ProjectState';
import { ShellState } from 'store/ShellState';
import { Welcome } from './Welcome';

ShellViewParticipants.register({
  id: 'welcome',
  isVisible() {
    if (ProjectState.hasProject) {
      return false;
    }

    return ShellState.isReady;
  },
  render: () => <Welcome />,
});
