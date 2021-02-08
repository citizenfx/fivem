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
import { entriesSorter, ProjectItemProps, ProjectItemRenderer } from './item';
import { ProjectExplorerContextProvider } from './ProjectExplorer.context';
import { PackAsset } from '../assets/PackAsset/PackAsset';
import s from './ProjectExplorer.module.scss';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';


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
    directoryCreatorOpen,
    closeDirectoryCreator,
    setResourceCreatorDir,
    openResourceCreator,
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

  const handleOpenResourceCreator = React.useCallback(() => {
    setResourceCreatorDir(project.path);
    openResourceCreator();
  }, [project.path, setResourceCreatorDir, openResourceCreator]);

  const nodes = project.fs[project.path]
    .filter(fsTreeFilter)
    .sort(entriesSorter)
    .map((entry) => itemRenderer({
      entry,
      project,
      pathsMap: project.fs,
      itemRenderer,
      creatorClassName: s.creator,
    }));

  const contextItems: ContextMenuItem[] = React.useMemo(() => {
    return [
      {
        id: 'new-resource',
        icon: newResourceIcon,
        text: 'New resource',
        onClick: handleOpenResourceCreator,
      },
      {
        id: 'new-directory',
        icon: newDirectoryIcon,
        text: 'New directory',
        onClick: openDirectoryCreator,
      },
    ];
  }, [handleOpenResourceCreator, openDirectoryCreator]);

  return (
    <ProjectExplorerContextProvider>
      <ContextMenu
        className={s.root}
        items={contextItems}
      >
        <ScrollContainer>
          {directoryCreatorOpen && (
            <DirectoryCreator
              className={s.creator}
              onCreate={handleDirectoryCreate}
            />
          )}
          <DndProvider backend={HTML5Backend}>
            {nodes}
          </DndProvider>
        </ScrollContainer>
      </ContextMenu>
    </ProjectExplorerContextProvider>
  );
});
