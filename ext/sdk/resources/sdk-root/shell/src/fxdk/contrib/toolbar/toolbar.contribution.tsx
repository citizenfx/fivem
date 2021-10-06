import React from 'react';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ProjectState } from 'store/ProjectState';
import { Toolbar } from './Toolbar';

ShellViewParticipants.register({
  id: 'toolbar',
  isVisible: () => ProjectState.hasProject,
  render: () => <Toolbar />,
});
