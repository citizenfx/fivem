import React from 'react';
import { observer } from 'mobx-react-lite';
import { ResourceCreator } from 'assets/resource/renderer/ResourceCreator/ResourceCreator';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import { ProjectSettings } from './ProjectSettings/ProjectSettings';
import { ProjectBuilder } from './ProjectBuilder/ProjectBuilder';
import { Importer } from './Importer/Importer';
import { ProjectState } from 'store/ProjectState';
import { ProjectCreator } from './ProjectCreator/ProjectCreator';
import { ProjectOpener } from './ProjectOpener/ProjectOpener';
import s from './Project.module.scss';


export const Project = observer(function Project() {
  const {
    builderOpen,
    settingsOpen,
    importerOpen,
    creatorOpen,
    openerOpen,
    resourceCreatorOpen,
  } = ProjectState;

  return (
    <>
      {creatorOpen && (
        <ProjectCreator />
      )}

      {openerOpen && (
        <ProjectOpener />
      )}

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
