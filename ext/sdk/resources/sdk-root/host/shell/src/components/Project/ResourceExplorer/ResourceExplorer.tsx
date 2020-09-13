import React from 'react';
import { ProjectContext } from '../../../contexts/ProjectContext';
import { assetKinds } from '../../../sdkApi/api.types';
import { projectApi } from '../../../sdkApi/events';
import { sendApiMessage } from '../../../utils/api';
import { invariant } from '../../../utils/invariant';
import { PackAsset } from '../assets/PackAsset/PackAsset';
import { Directory } from './Directory/Directory';
import { File } from './File/File';
import { DirectoryCreator } from './Directory/DirectoryCreator/DirectoryCreator';
import { Resource } from './Resource/Resource';
import { ProjectItemProps, ProjectItemRenderer } from './ResourceExplorer.item';
import s from './ResourceExplorer.module.scss';


const assetTypeRenderers = {
  [assetKinds.pack]: PackAsset,
};

const itemRenderer: ProjectItemRenderer = (props: ProjectItemProps) => {
  const { entry } = props;

  if (entry.isFile) {
    return (
      <File
        key={entry.path}
        {...props}
      />
    );
  }

  if (entry.meta.isResource) {
    return (
      <Resource
        key={entry.path}
        {...props}
      />
    );
  }

  if (entry.isDirectory) {
    if (entry.meta.assetMeta) {
      const AssetRenderer = assetTypeRenderers[entry.meta.assetMeta.kind];

      return (
        <AssetRenderer
          key={entry.path}
          {...props}
        />
      );
    }

    return (
      <Directory
        key={entry.path}
        {...props}
      />
    );
  }
};

export const ResourceExplorer = React.memo(() => {
  const { project, projectResources, directoryCreatorOpen, closeDirectoryCreator } = React.useContext(ProjectContext);
  invariant(project, `ResourceExplorer was rendered without project set`);

  const handleDirectoryCreate = React.useCallback((directoryName: string) => {
    closeDirectoryCreator();

    if (directoryName) {
      sendApiMessage(projectApi.createDirectory, {
        projectPath: project.path,
        directoryPath: project.path,
        directoryName,
      });
    }
  }, [project, closeDirectoryCreator]);

  const nodes = project.fsTree.entries
    .filter((entry) => {
      if (entry.name === '.fxserver') {
        return false;
      }

      if (entry.name === '.fxdk') {
        return false;
      }

      if (entry.isFile) {
        return false;
      }

      return true;
    })
    .map((entry) => itemRenderer({
      entry,
      project,
      projectResources,
      pathsMap: project.fsTree.pathsMap,
      itemRenderer,
      creatorClassName: s.creator,
    }));

  return (
    <div className={s.root}>
      {directoryCreatorOpen && (
        <DirectoryCreator
          className={s.creator}
          onCreate={handleDirectoryCreate}
        />
      )}
      {nodes}
    </div>
  );
});
