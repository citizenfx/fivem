import { Button } from 'components/controls/Button/Button';
import { closeIcon, projectIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';
import * as React from 'react';
import { projectApi } from 'shared/api.events';
import { RecentProject } from 'shared/api.types';
import { sendApiMessage } from 'utils/api';
import s from './RecentProjectItem.module.scss';

export interface RecentProjectItemProps {
  recentProject: RecentProject,
}

export const RecentProjectItem = React.memo(function RecentProjectItem({ recentProject }: RecentProjectItemProps) {
  const { openProject: baseOpenProject, closeOpener } = React.useContext(ProjectContext);

  const openProject = React.useCallback(() => {
    baseOpenProject(recentProject.path);
    closeOpener();
  }, [recentProject.path, baseOpenProject, closeOpener]);

  const removeRecentProject = React.useCallback(() => {
    sendApiMessage(projectApi.removeRecent, recentProject.path);
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
          theme="transparent"
          icon={closeIcon}
          onClick={removeRecentProject}
          title="Remove recent project from the list"
        />
      </div>
    </div>
  );
});
