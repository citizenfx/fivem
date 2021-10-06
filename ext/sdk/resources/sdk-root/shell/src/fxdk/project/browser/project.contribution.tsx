import React from 'react';
import { IToolbarTitleViewParticipant, IToolbarViewParticipant, ToolbarParticipants } from 'fxdk/contrib/toolbar/toolbarExtensions';
import { ProjectView } from './ProjectView';
import { ShellState } from 'store/ShellState';
import { ProjectControls } from './ProjectControls/ProjectControls';
import { ProjectState } from 'store/ProjectState';

ToolbarParticipants.registerView(new class ProjectViewParticipant implements IToolbarViewParticipant {
  readonly id = 'project-view';

  render() {
    return (
      <ProjectView />
    );
  }
});

ToolbarParticipants.registerTitleView(new class ProjectTitleViewParticipant implements IToolbarTitleViewParticipant {
  readonly id = 'project-title-view';

  isVisible() {
    return ShellState.isMainPersonality;
  }

  render() {
    return (
      <ProjectControls />
    );
  }
});

ToolbarParticipants.registerMenuItem({
  id: 'project-close',
  order: Number.MAX_SAFE_INTEGER,
  group: 'project-switch',
  item: {
    id: 'project-close',
    text: 'Close project',
    onClick: ProjectState.closeProject,
  },
});
