import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { closeIcon, projectIcon } from 'fxdk/ui/icons';
import { RecentProject } from 'shared/project.types';
import { ShellCommands } from 'shell-api/commands';
import { ProjectOpenerCommands } from 'fxdk/project/contrib/opener/opener.commands';
import { Api } from 'fxdk/browser/Api';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import { ProjectApi } from 'fxdk/project/common/project.api';
import s from './RecentProjectItem.module.scss';

export interface RecentProjectItemProps {
  recentProject: RecentProject,
}

export const RecentProjectItem = React.memo(function RecentProjectItem({ recentProject }: RecentProjectItemProps) {
  const openProject = React.useCallback(() => {
    ProjectLoader.open(recentProject.path);
    ShellCommands.invoke(ProjectOpenerCommands.CLOSE);
  }, [recentProject.path]);

  const removeRecentProject = React.useCallback(() => {
    Api.send(ProjectApi.LoaderEndpoints.removeRecent, recentProject.path);
  }, [recentProject.path]);

  return (
    <div className={s.root}>
      <div className={s.content} onClick={openProject}>
        <div className={s.icon}>
          {projectIcon}
        </div>

        <div className={s.info}>
          <div className={s.name}>
            {recentProject.name}
          </div>
          <div className={s.path}>
            {recentProject.path}
          </div>
        </div>
      </div>

      <div className={s.actions}>
        <Button
          icon={closeIcon}
          onClick={removeRecentProject}
          title="Remove recent project from the list"
        />
      </div>
    </div>
  );
});
