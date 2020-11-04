import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { RootsExplorer } from 'components/Explorer/Explorer';
import { Modal } from 'components/Modal/Modal';
import { ProjectContext } from 'contexts/ProjectContext';
import { FilesystemEntry } from 'sdkApi/api.types';
import { projectApi } from 'sdkApi/events';
import { sendApiMessage } from 'utils/api';
import s from './ProjectOpener.module.scss';


const selectableFilter = (entry: FilesystemEntry): boolean => {
  return !!entry.meta.isFxdkProject;
};

export const ProjectOpener = React.memo(function ProjectOpener() {
  const { project, recentProjects, closeOpener } = React.useContext(ProjectContext);
  const [projectPath, setProjectPath] = React.useState<string>();

  const openProject = React.useCallback((projectPath) => {
    sendApiMessage(projectApi.open, projectPath);
    closeOpener();
  }, [closeOpener]);

  const openSelectedProject = React.useCallback(() => {
    if (projectPath) {
      sendApiMessage(projectApi.open, projectPath);
      closeOpener();
    }
  }, [closeOpener, projectPath]);

  const removeRecentProject = React.useCallback((projectPath) => {
    sendApiMessage(projectApi.removeRecent, projectPath);
  }, []);

  const recents = recentProjects
    // No need to show current open porject
    .filter((recentProject) => project?.path !== recentProject.path)
    .map((recentProject) => (
      <div key={recentProject.path} className={s['recent-project']}>
        <div className={s.info} onClick={() => openProject(recentProject.path)}>
          <div className={s.name}>
            {recentProject.name}
          </div>
          <div className={s.path}>
            {recentProject.path}
          </div>
        </div>
        <div className={s.actions}>
          <Button
            theme="transparent"
            text="Remove"
            onClick={() => removeRecentProject(recentProject.path)}
          />
        </div>
      </div>
    ));

  return (
    <Modal onClose={closeOpener}>
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
        <RootsExplorer
          hideFiles
          selectedPath={projectPath}
          onSelectPath={setProjectPath}
          selectableFilter={selectableFilter}
        />

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Open selected project"
            disabled={!projectPath}
            onClick={openSelectedProject}
          />
          <Button
            text="Cancel"
            onClick={closeOpener}
          />
        </div>
      </div>
    </Modal>
  );
});
