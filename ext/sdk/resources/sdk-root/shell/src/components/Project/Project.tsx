import React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { ResourceCreator } from './ProjectExplorer/Resource/ResourceCreator/ResourceCreator';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import s from './Project.module.scss';


export const Project = React.memo(function Project() {
  const {
    project,
    resourceCreatorOpen: assetCreatorOpen,
  } = React.useContext(ProjectContext);

  const showProjectExplorer = !!project?.fs[project?.path];

  return (
    <>
      {assetCreatorOpen && (
        <ResourceCreator />
      )}

      <div className={s.root}>
        {showProjectExplorer && (
          <ProjectExplorer />
        )}
      </div>
    </>
  );
});
