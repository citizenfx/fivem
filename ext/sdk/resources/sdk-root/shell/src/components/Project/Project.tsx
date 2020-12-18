import React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { resourceIcon } from 'constants/icons';
import { Server } from 'components/Server/Server';
import { AssetCreator } from './AssetCreator/AssetCreator';
import { ProjectExplorer } from './ProjectExplorer/ProjectExplorer';
import s from './Project.module.scss';


export const Project = React.memo(function Project() {
  const {
    project,
    assetCreatorOpen,
  } = React.useContext(ProjectContext);

  const showProjectExplorer = !!project?.fs[project?.path];

  return (
    <>
      {assetCreatorOpen && (
        <AssetCreator />
      )}

      <div className={s.root}>
        {showProjectExplorer && (
          <ProjectExplorer />
        )}
      </div>
    </>
  );
});
