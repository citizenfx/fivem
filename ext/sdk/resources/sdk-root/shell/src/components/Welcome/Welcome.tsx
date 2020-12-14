import React from 'react';
import classnames from 'classnames';
import { ProjectContext } from 'contexts/ProjectContext';
import { Button } from 'components/controls/Button/Button';
import s from './Welcome.module.scss';
import { RecentProjectItem } from 'components/RecentProjectItem/RecentProjectItem';


export const Welcome = React.memo(function Welcome() {
  const { recentProjects, openProject, openCreator, openOpener } = React.useContext(ProjectContext);

  const [active, setActive] = React.useState(false);
  React.useEffect(() => {
    const timer = setTimeout(() => setActive(true), 500);

    return () => clearTimeout(timer);
  }, []);

  return (
    <div className={classnames(s.root, 'animated-background', { [s.active]: active })}>
      <div className={s.header}>
        <h1>
          Welcome to FxDK
        </h1>
        <span>
          The present and the future of development for cfx.re platform
        </span>
      </div>

      {!!recentProjects.length && (
        <div className={s['recent-projects']}>
          {recentProjects.map((recentProject) => (
            <RecentProjectItem
              key={recentProject.path}
              recentProject={recentProject}
            />
          ))}
        </div>
      )}

      <div className={s.controls}>
        <Button
          text="Create New Project"
          theme="primary"
          onClick={openCreator}
        />
        <Button
          text="Open Project"
          onClick={openOpener}
        />
      </div>
    </div>
  );
});
