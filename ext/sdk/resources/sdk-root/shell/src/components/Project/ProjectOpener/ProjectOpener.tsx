import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { RootsExplorer } from 'components/Explorer/Explorer';
import { Modal } from 'components/Modal/Modal';
import { ProjectContext } from 'contexts/ProjectContext';
import { FilesystemEntry } from 'shared/api.types';
import { RecentProjectItem } from 'components/RecentProjectItem/RecentProjectItem';
import s from './ProjectOpener.module.scss';


const selectableFilter = (entry: FilesystemEntry): boolean => {
  return !!entry.meta.isFxdkProject;
};

export const ProjectOpener = React.memo(function ProjectOpener() {
  const { project, recentProjects, closeOpener, openProject: baseOpenProject } = React.useContext(ProjectContext);
  const [projectPath, setProjectPath] = React.useState<string>();

  const openSelectedProject = React.useCallback(() => {
    if (projectPath) {
      baseOpenProject(projectPath);
      closeOpener();
    }
  }, [closeOpener, projectPath, baseOpenProject]);

  const recents = recentProjects
    // No need to show current open porject
    .filter((recentProject) => project?.path !== recentProject.path)
    .map((recentProject) => (
      <RecentProjectItem
        key={recentProject.path}
        recentProject={recentProject}
      />
    ));

  return (
    <Modal fullWidth onClose={closeOpener}>
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
