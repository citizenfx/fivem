import React from 'react';
import { DndProvider } from 'react-dnd';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { assetKinds } from 'shared/api.types';
import { ProjectContext } from 'contexts/ProjectContext';
import { invariant } from 'utils/invariant';
import { sendApiMessage } from 'utils/api';
import { projectApi } from 'shared/api.events';
import { ContextMenu, ContextMenuItem } from 'components/controls/ContextMenu/ContextMenu';
import { newDirectoryIcon, newResourceIcon } from 'constants/icons';
import { Directory } from './Directory/Directory';
import { File } from './File/File';
import { DirectoryCreator } from './Directory/DirectoryCreator/DirectoryCreator';
import { Resource } from './Resource/Resource';
import { ProjectItemProps, ProjectItemRenderer } from './ProjectExplorer.item';
import { ProjectExplorerContextProvider } from './ProjectExplorer.context';
import { PackAsset } from '../assets/PackAsset/PackAsset';
import s from './ProjectExplorer.module.scss';


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

const fsTreeFilter = (entry) => {
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
};

export const ProjectExplorer = React.memo(function ProjectExplorer() {
  const {
    project,
    projectResources,
    directoryCreatorOpen,
    closeDirectoryCreator,
    setAssetCreatorDir,
    openAssetCreator,
    openDirectoryCreator,
  } = React.useContext(ProjectContext);
  invariant(project, `ProjectExplorer was rendered without project set`);

  const handleDirectoryCreate = React.useCallback((directoryName: string) => {
    closeDirectoryCreator();

    if (directoryName) {
      sendApiMessage(projectApi.createDirectory, {
        directoryPath: project.path,
        directoryName,
      });
    }
  }, [project.path, closeDirectoryCreator]);

  const handleOpenAssetCreator = React.useCallback(() => {
    setAssetCreatorDir(project.path);
    openAssetCreator();
  }, [project.path, setAssetCreatorDir, openAssetCreator]);

  const nodes = project.fs[project.path]
    .filter(fsTreeFilter)
    .map((entry) => itemRenderer({
      entry,
      project,
      projectResources,
      pathsMap: project.fs,
      itemRenderer,
      creatorClassName: s.creator,
    }));

  const contextItems: ContextMenuItem[] = React.useMemo(() => {
    return [
      {
        id: 'new-asset',
        icon: newResourceIcon,
        text: 'New asset',
        onClick: handleOpenAssetCreator,
      },
      {
        id: 'new-directory',
        icon: newDirectoryIcon,
        text: 'New directory',
        onClick: openDirectoryCreator,
      },
    ];
  }, [handleOpenAssetCreator, openDirectoryCreator]);

  return (
    <ProjectExplorerContextProvider>
      <ContextMenu
        className={s.root}
        items={contextItems}
      >
        {directoryCreatorOpen && (
          <DirectoryCreator
            className={s.creator}
            onCreate={handleDirectoryCreate}
          />
        )}
        <DndProvider backend={HTML5Backend}>
          {nodes}
        </DndProvider>
      </ContextMenu>
    </ProjectExplorerContextProvider>
  );
});
