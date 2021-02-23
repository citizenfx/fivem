import React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { ResourceCreator } from './ProjectExplorer/Resource/ResourceCreator/ResourceCreator';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import { ProjectSettings } from './ProjectSettings/ProjectSettings';
import { ProjectBuilder } from './ProjectBuilder/ProjectBuilder';
import { Importer } from './Importer/Importer';
import s from './Project.module.scss';


export const Project = React.memo(function Project() {
  const {
    project,
    builderOpen,
    settingsOpen,
    importerOpen,
    resourceCreatorOpen,
  } = React.useContext(ProjectContext);

  const showProjectExplorer = !!project?.fs[project?.path];

  return (
    <>
      {settingsOpen && (
        <ProjectSettings />
      )}

      {builderOpen && (
        <ProjectBuilder />
      )}

      {importerOpen && (
        <Importer />
      )}

      {resourceCreatorOpen && (
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
