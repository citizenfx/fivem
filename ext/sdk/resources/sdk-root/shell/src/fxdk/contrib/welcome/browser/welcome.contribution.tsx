import React from 'react';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import { ShellState } from 'store/ShellState';
import { Welcome } from './Welcome';

ShellViewParticipants.register({
  id: 'welcome',
  isVisible() {
    if (ProjectLoader.hasProject) {
      return false;
    }

    return ShellState.isReady;
  },
  render: () => <Welcome />,
});
