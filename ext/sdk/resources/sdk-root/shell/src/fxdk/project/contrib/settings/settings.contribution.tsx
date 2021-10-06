import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ProjectSettings } from './ProjectSettings';
import { ShellCommands } from 'shell-api/commands';
import { ProjectSettingsCommands } from './settings.commands';
import { SettingsState } from './SettingsState';
import { ToolbarParticipants } from 'fxdk/contrib/toolbar/toolbarExtensions';
import { projectSettingsIcon } from 'constants/icons';

ShellCommands.register(ProjectSettingsCommands.OPEN, SettingsState.open);

ProjectParticipants.registerRender({
  id: 'settings-render',
  isVisible: () => SettingsState.isOpen,
  render: () => <ProjectSettings />,
});

ToolbarParticipants.registerMenuItem({
  id: 'project-settings',
  group: 'project-main',
  order: 0,
  item: {
    id: 'project-settings',
    text: 'Project settings',
    icon: projectSettingsIcon,
    onClick: SettingsState.open,
  },
});
