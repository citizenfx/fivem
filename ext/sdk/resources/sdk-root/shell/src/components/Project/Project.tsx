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
import { StatusState } from 'store/StatusState';
import { Feature } from 'shared/api.types';
import { DndProvider } from 'react-dnd';
import { HTML5Backend } from 'react-dnd-html5-backend';
import s from './Project.module.scss';


export const Project = observer(function Project() {
  const worldEditorAvailable = StatusState.getFeature(Feature.worldEditor);

  return (
    <>
      {ProjectState.creatorUI.isOpen && (
        <ProjectCreator />
      )}

      {ProjectState.openerUI.isOpen && (
        <ProjectOpener />
      )}

      {ProjectState.hasProject && ProjectState.settingsUI.isOpen && (
        <ProjectSettings />
      )}

      {ProjectState.hasProject && ProjectState.builderUI.isOpen && (
        <ProjectBuilder />
      )}

      {ProjectState.hasProject && ProjectState.importerUI.isOpen && (
        <Importer />
      )}

      {ProjectState.hasProject && ProjectState.resourceCreatorUI.isOpen && (
        <ResourceCreator />
      )}

      {worldEditorAvailable && ProjectState.hasProject && ProjectState.mapCreatorUI.isOpen && (
        <FXWorldCreator />
      )}

      <div className={s.root}>
        {ProjectState.hasProject && (
          <DndProvider backend={HTML5Backend}>
            <ProjectExplorer />
          </DndProvider>
        )}
      </div>
    </>
  );
});
