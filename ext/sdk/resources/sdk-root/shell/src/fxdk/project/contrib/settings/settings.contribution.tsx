import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ProjectSettings } from './ProjectSettings';
import { ShellCommands } from 'shell-api/commands';
import { ProjectSettingsCommands } from './settings.commands';
import { SettingsState } from './SettingsState';
import { ToolbarParticipants } from 'fxdk/contrib/toolbar/browser/toolbarExtensions';
import { projectSettingsIcon } from 'fxdk/ui/icons';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';

ShellCommands.register(ProjectSettingsCommands.OPEN, SettingsState.open);

ProjectParticipants.registerRender({
  id: 'settings-render',
  isVisible: () => ProjectLoader.hasProject && SettingsState.isOpen,
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
