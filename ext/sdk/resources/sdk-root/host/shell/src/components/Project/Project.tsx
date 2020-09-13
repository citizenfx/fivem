import React from 'react';
import { AssetCreator } from './AssetCreator/AssetCreator';
import { ProjectContext } from '../../contexts/ProjectContext';
import { ResourceExplorer } from './ResourceExplorer/ResourceExplorer';
import { newDirectoryIcon, newResourceIcon, projectConfigIcon, resourceIcon } from '../../constants/icons';
import { Server } from '../Server/Server';
import s from './Project.module.scss';


export const Project = React.memo(() => {
  const {
    project,
    projectResources,
    setAssetCreatorDir,
    assetCreatorOpen,
    openAssetCreator,
    openDirectoryCreator,
  } = React.useContext(ProjectContext);

  const handleCreateResource = React.useCallback(() => {
    if (project) {
      setAssetCreatorDir(project.path);
      openAssetCreator();
    }
  }, [project, setAssetCreatorDir, openAssetCreator]);

  const showResourceExplorer = !!project?.fsTree.entries.length;

  const resourcesCount = Object.keys(projectResources).length;

  return (
    <>
      {assetCreatorOpen && (
        <AssetCreator />
      )}

      <div className={s.root}>
        <div className={s.row}>
          <div className={s.title}>
            <span className={s.name}>
              {project?.manifest.name || ''}
            </span>
          </div>

          <div className={s.extra}>
            <div className={s.stats}>
              <span title={`Resources in this project: ${resourcesCount}`}>
                {resourceIcon} {resourcesCount}
              </span>
            </div>

            <div className={s.server}>
              <Server />
            </div>

            <button className={s.config}>
              {projectConfigIcon}
            </button>
          </div>
        </div>

        <div className={s.controls}>
          <div className={s.group}>
            <button disabled={!project} onClick={handleCreateResource}>
              {newResourceIcon}
              New Resource
            </button>
            <button disabled={!project} onClick={openDirectoryCreator}>
              {newDirectoryIcon}
              Add directory
            </button>
          </div>
        </div>

        {showResourceExplorer && (
          <ResourceExplorer />
        )}
      </div>
    </>
  );
});
