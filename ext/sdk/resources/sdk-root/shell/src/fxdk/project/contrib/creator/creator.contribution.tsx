import React from 'react';
import { ProjectCreator } from './ProjectCreator';
import { ShellCommands } from 'shell-api/commands';
import { ProjectCreatorState } from './ProjectCreatorState';
import { ToolbarParticipants } from 'fxdk/contrib/toolbar/toolbarExtensions';
import { newProjectIcon } from 'constants/icons';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ProjectCreatorCommands } from './creator.commands';

ShellCommands.register(ProjectCreatorCommands.OPEN, ProjectCreatorState.open);

ShellViewParticipants.register({
  id: 'creator-render',
  isVisible: () => ProjectCreatorState.isOpen,
  render: () => <ProjectCreator />,
});

ToolbarParticipants.registerMenuItem({
  id: 'project-creator',
  group: 'project-switch',
  order: 1,
  item: {
    id: 'new-project',
    text: 'New Project',
    icon: newProjectIcon,
    onClick: ProjectCreatorState.open,
  },
});
