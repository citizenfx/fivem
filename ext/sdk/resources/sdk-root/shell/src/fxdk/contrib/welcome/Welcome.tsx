import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { renderViewRegistryItems } from 'fxdk/base/viewRegistry';
import { WelcomeParticipants } from './welcomeExtensions';
import { ShellCommands } from 'shell-api/commands';
import { ProjectCreatorCommands } from 'fxdk/project/contrib/creator/creator.commands';
import { ProjectOpenerCommands } from 'fxdk/project/contrib/opener/opener.commands';
import s from './Welcome.module.scss';
import { ProjectState } from 'store/ProjectState';
import { TaskState } from 'store/TaskState';
import { projectCreatingTaskName, projectLoadingTaskName } from 'shared/task.names';
import { Indicator } from 'fxdk/ui/Indicator/Indicator';

function showLoader(): boolean {
  if (ProjectState.areRecentProjectsLoading) {
    return true;
  }

  if (ProjectState.isProjectOpening) {
    return true;
  }

  if (TaskState.get(projectLoadingTaskName)) {
    return true;
  }

  if (TaskState.get(projectCreatingTaskName)) {
    return true;
  }

  return false;
}

export const Welcome = observer(function Welcome() {
  if (showLoader()) {
    return (
      <div className={s.root}>
        <Indicator />
      </div>
    );
  }

  const viewNodes = renderViewRegistryItems(WelcomeParticipants.getAllVisibleViews());

  return (
    <div className={classnames(s.root, 'animated-background')}>
      <div className={s.header}>
        <h1>
          Welcome to FxDK
        </h1>
        <span>
          The reordered future of Cfx.re platform development
        </span>
      </div>

      {viewNodes}

      <div className={s.controls}>
        <Button
          text="Create New Project"
          theme="primary"
          onClick={() => ShellCommands.invoke(ProjectCreatorCommands.OPEN)}
        />
        <Button
          text="Open Project"
          onClick={() => ShellCommands.invoke(ProjectOpenerCommands.OPEN)}
        />
      </div>
    </div>
  );
});
