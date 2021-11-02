import React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { RecentProjectItem } from 'fxdk/project/contrib/opener/RecentProjectItem/RecentProjectItem';
import { useSendApiMessageCallback } from 'utils/hooks';
import { PathSelector } from 'fxdk/ui/controls/PathSelector/PathSelector';
import { ProjectOpenerState } from './ProjectOpenerState';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import { Project } from 'fxdk/project/browser/state/project';
import s from './ProjectOpener.module.scss';
import { ProjectApi } from 'fxdk/project/common/project.api';

export const ProjectOpener = observer(function ProjectOpener() {
  const currentProjectPath = ProjectLoader.hasProject
    ? Project.path
    : null;

  const [projectPath, setProjectPath] = React.useState<string>('');

  const [projectPathChecking, setProjectPathChecking] = React.useState(false);
  const [projectPathOpenable, setProjectPathOpenable] = React.useState(true);

  const checkProjectPath = useSendApiMessageCallback<string, boolean>(ProjectApi.LoaderEndpoints.checkOpenRequest, (error, result) => {
    setProjectPathChecking(false);
    setProjectPathOpenable(result && !error);
  });

  const openSelectedProject = React.useCallback(() => {
    if (projectPath && projectPathOpenable) {
      ProjectLoader.open(projectPath);
      ProjectOpenerState.close();
    }
  }, [projectPath, projectPathOpenable]);

  const handleProjectPathChange = React.useCallback((newProjectPath) => {
    setProjectPathChecking(true);
    setProjectPath(newProjectPath);
    checkProjectPath(newProjectPath);
  }, [setProjectPath, checkProjectPath, setProjectPathChecking]);

  const recents = ProjectLoader.recentProjects
    // No need to show current open porject
    .filter((recentProject) => currentProjectPath !== recentProject.path)
    .map((recentProject) => (
      <RecentProjectItem
        key={recentProject.path}
        recentProject={recentProject}
      />
    ));

  return (
    <Modal onClose={ProjectOpenerState.close}>
      <div className={s.root}>
        <div className="modal-header">
          Open project
        </div>

        {!!recents.length && (
          <>
            <div className={s.label}>
              Recent projects
            </div>

            <div className={s.recents}>
              {recents}
            </div>
          </>
        )}

        <div className={s.label}>
          Choose project from filesystem
        </div>
        <div className="modal-block">
          <PathSelector
            value={projectPath}
            onChange={handleProjectPathChange}
            showLoader={projectPathChecking}
            description={!projectPathOpenable && `Path doesn't look like an FxDK project`}
          />
        </div>

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Open selected project"
            disabled={!projectPath || !projectPathOpenable}
            onClick={openSelectedProject}
          />
          <Button
            text="Cancel"
            onClick={ProjectOpenerState.close}
          />
        </div>
      </div>
    </Modal>
  );
});
