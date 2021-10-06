import React from 'react';
import { ProjectOpener } from './ProjectOpener';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ShellCommands } from 'shell-api/commands';
import { ProjectOpenerCommands } from './opener.commands';
import { ProjectOpenerState } from './ProjectOpenerState';
import { ToolbarParticipants } from 'fxdk/contrib/toolbar/toolbarExtensions';
import { openProjectIcon } from 'constants/icons';
import { WelcomeParticipants } from 'fxdk/contrib/welcome/welcomeExtensions';
import { ProjectState } from 'store/ProjectState';
import { WelcomeView } from './WelcomeView';

ShellCommands.register(ProjectOpenerCommands.OPEN, ProjectOpenerState.open);
ShellCommands.register(ProjectOpenerCommands.CLOSE, ProjectOpenerState.close);

ShellViewParticipants.register({
  id: 'project-opener',
  isVisible: () => ProjectOpenerState.isOpen,
  render: () => <ProjectOpener />,
});

ToolbarParticipants.registerMenuItem({
  id: 'project-opener',
  group: 'project-switch',
  order: 0,
  item: {
    id: 'open-project',
    text: 'Open Project',
    icon: openProjectIcon,
    onClick: ProjectOpenerState.open,
  },
});

WelcomeParticipants.regsiterView({
  id: 'recent-projects',
  isVisible: () => ProjectState.recentProjects.length > 0,
  render: () => <WelcomeView />,
});
