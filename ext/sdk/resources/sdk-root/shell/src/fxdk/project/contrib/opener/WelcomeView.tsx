import React from 'react';
import { observer } from 'mobx-react-lite';
import { RecentProjectItem } from './RecentProjectItem/RecentProjectItem';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import s from './WelcomeView.module.scss';

export const WelcomeView = observer(function WelcomeView() {
  const recentProjectNodes = ProjectLoader.recentProjects.map((recentProject) => (
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
