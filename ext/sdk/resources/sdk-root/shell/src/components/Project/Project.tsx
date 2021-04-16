import React from 'react';
import { ResourceCreator } from './ProjectExplorer/Resource/ResourceCreator/ResourceCreator';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import { ProjectSettings } from './ProjectSettings/ProjectSettings';
import { ProjectBuilder } from './ProjectBuilder/ProjectBuilder';
import { Importer } from './Importer/Importer';
import { ProjectState } from 'store/ProjectState';
import { observer } from 'mobx-react-lite';
import s from './Project.module.scss';


export const Project = observer(function Project() {
  const {
    builderOpen,
    settingsOpen,
    importerOpen,
    resourceCreatorOpen,
  } = ProjectState;

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
        {ProjectState.hasProject && (
          <ProjectExplorer />
        )}
      </div>
    </>
  );
});
