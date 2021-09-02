import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { Button } from 'components/controls/Button/Button';
import { RecentProjectItem } from 'components/RecentProjectItem/RecentProjectItem';
import { ProjectState } from 'store/ProjectState';
import s from './Welcome.module.scss';


export const Welcome = observer(function Welcome() {
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
          The reordered future of Cfx.re platform development
        </span>
      </div>

      {!!ProjectState.recentProjects.length && (
        <div className={s['recent-projects']}>
          {ProjectState.recentProjects.map((recentProject) => (
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
          onClick={ProjectState.creatorUI.open}
        />
        <Button
          text="Open Project"
          onClick={ProjectState.openerUI.open}
        />
      </div>
    </div>
  );
});
