import React from 'react';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { Toolbar } from './Toolbar';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';

ShellViewParticipants.register({
  id: 'toolbar',
  isVisible: () => ProjectLoader.hasProject,
  render: () => <Toolbar />,
});
