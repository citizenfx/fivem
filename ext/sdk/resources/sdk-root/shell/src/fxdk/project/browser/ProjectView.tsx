import React from 'react';
import { observer } from 'mobx-react-lite';
import { DndProvider } from 'react-dnd';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import { ProjectParticipants } from './projectExtensions';
import { renderViewRegistryItems } from 'fxdk/base/viewRegistry';
import s from './Project.module.scss';

export const ProjectView = observer(function ProjectView() {
  const renderNodes = renderViewRegistryItems(ProjectParticipants.getAllVisibleRenders());

  return (
    <div className={s.root}>
      <DndProvider backend={HTML5Backend}>
        {renderNodes}

        <ProjectExplorer />
      </DndProvider>
    </div>
  );
});
