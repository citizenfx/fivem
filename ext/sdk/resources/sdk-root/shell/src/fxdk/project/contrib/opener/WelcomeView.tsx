import React from 'react';
import { observer } from 'mobx-react-lite';
import { ProjectState } from 'store/ProjectState';
import { RecentProjectItem } from './RecentProjectItem/RecentProjectItem';
import s from './WelcomeView.module.scss';

export const WelcomeView = observer(function WelcomeView() {
  const recentProjectNodes = ProjectState.recentProjects.map((recentProject) => (
    <RecentProjectItem
      key={recentProject.path}
      recentProject={recentProject}
    />
  ));

  return (
    <div className={s.root}>
      {recentProjectNodes}
    </div>
  );
});
