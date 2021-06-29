import React from 'react';
import { observer } from 'mobx-react-lite';
import { ResourceCreator } from 'assets/resource/renderer/ResourceCreator/ResourceCreator';
import { FXWorldCreator } from 'assets/fxworld/renderer/FXWorldCreator/FXWorldCreator';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import { ProjectSettings } from './ProjectSettings/ProjectSettings';
import { ProjectBuilder } from './ProjectBuilder/ProjectBuilder';
import { Importer } from './Importer/Importer';
import { ProjectState } from 'store/ProjectState';
import { ProjectCreator } from './ProjectCreator/ProjectCreator';
import { ProjectOpener } from './ProjectOpener/ProjectOpener';
import s from './Project.module.scss';
import { StatusState } from 'store/StatusState';
import { Feature } from 'shared/api.types';


export const Project = observer(function Project() {
  const {
    builderOpen,
    settingsOpen,
    importerOpen,
    creatorOpen,
    openerOpen,
    resourceCreatorOpen,
    mapCreatorOpen,
  } = ProjectState;

  const worldEditorAvailable = StatusState.getFeature(Feature.worldEditor);

  return (
    <>
      {creatorOpen && (
        <ProjectCreator />
      )}

      {openerOpen && (
        <ProjectOpener />
      )}

      {ProjectState.hasProject && settingsOpen && (
        <ProjectSettings />
      )}

      {ProjectState.hasProject && builderOpen && (
        <ProjectBuilder />
      )}

      {ProjectState.hasProject && importerOpen && (
        <Importer />
      )}

      {ProjectState.hasProject && resourceCreatorOpen && (
        <ResourceCreator />
      )}

      {worldEditorAvailable && ProjectState.hasProject && mapCreatorOpen && (
        <FXWorldCreator />
      )}

      <div className={s.root}>
        {ProjectState.hasProject && (
          <ProjectExplorer />
        )}
      </div>
    </>
  );
});
