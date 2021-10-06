import React from 'react';
import { ProjectParticipants } from 'fxdk/project/browser/projectExtensions';
import { ShellCommands } from 'shell-api/commands';
import { ProjectState } from 'store/ProjectState';
import { ProjectBuilderCommands } from './builder.commands';
import { ProjectBuilder } from './ProjectBuilder';
import { projectBuildIcon } from 'constants/icons';
import { BuilderState } from './BuilderState';
import { ToolbarParticipants } from 'fxdk/contrib/toolbar/toolbarExtensions';

ShellCommands.register(ProjectBuilderCommands.BUILD, ProjectState.buildProject);
ShellCommands.register(ProjectBuilderCommands.OPEN, BuilderState.open);

ProjectParticipants.registerRender({
  id: 'project-builder-render',
  isVisible: () => BuilderState.isOpen,
  render: () => <ProjectBuilder />,
});

ProjectParticipants.registerControl({
  id: 'build',
  icon: projectBuildIcon,
  label: 'Build Project',
  commandId: ProjectBuilderCommands.BUILD,
  introId: 'project-build',
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
