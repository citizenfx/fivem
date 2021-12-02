import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import { ProjectBuilderCommands } from './builder.commands';
import { ProjectBuilder } from './ProjectBuilder';
import { projectBuildIcon, projectSettingsIcon } from 'fxdk/ui/icons';
import { BuilderState } from './BuilderState';
import { ToolbarParticipants } from 'fxdk/contrib/toolbar/browser/toolbarExtensions';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import { Project } from 'fxdk/project/browser/state/project';

ShellCommands.register(ProjectBuilderCommands.BUILD, (...args) => {
  if (ProjectLoader.hasProject) {
    Project.buildProject(...args);
  }
});
ShellCommands.register(ProjectBuilderCommands.OPEN, BuilderState.open);

ProjectParticipants.registerRender({
  id: 'project-builder-render',
  isVisible: () => BuilderState.isOpen,
  render: () => <ProjectBuilder />,
});

ProjectParticipants.registerControl({
  id: 'build',
  icon: projectBuildIcon,
  introId: 'project-build',
  controls: [
    {
      id: 'fast-build',
      icon: projectBuildIcon,
      label: 'Build Project',
      commandId: ProjectBuilderCommands.BUILD,
    },
    {
      id: 'open-build-modal',
      icon: projectSettingsIcon,
      label: 'Open build modal',
      commandId: ProjectBuilderCommands.OPEN,
    },
  ],
});

ToolbarParticipants.registerMenuItem({
  id: 'project-builder',
  group: 'project-main',
  order: 1,
  item: {
    id: 'project-builder',
    text: 'Build Project',
    icon: projectBuildIcon,
    onClick: BuilderState.open,
  },
});
