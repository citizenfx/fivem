import React from 'react';
import { observer } from 'mobx-react-lite';
import { DndProvider } from 'react-dnd';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { sendApiMessage } from 'utils/api';
import { projectApi } from 'shared/api.events';
import { ContextMenu, ContextMenuItem } from 'components/controls/ContextMenu/ContextMenu';
import { importAssetIcon, newDirectoryIcon, newResourceIcon } from 'constants/icons';
import { Directory } from './Directory/Directory';
import { File } from './File/File';
import { DirectoryCreator } from './Directory/DirectoryCreator/DirectoryCreator';
import { entriesSorter, ProjectItemProps, ProjectItemRenderer } from './item';
import { ProjectExplorerContextProvider } from './ProjectExplorer.context';
import { isAssetMetaFile } from 'utils/project';
import { ProjectState } from 'store/ProjectState';
import { ENABLED_ASSET_RENDERERS } from 'assets/enabled-renderers';
import { StatusState } from 'store/StatusState';
import { Feature } from 'shared/api.types';
import { assetTypes } from 'shared/asset.types';
import s from './ProjectExplorer.module.scss';
import { WiWindy } from 'react-icons/wi';


const itemRenderer: ProjectItemRenderer = (props: ProjectItemProps) => {
  const { entry } = props;
  const worldEditorEnabled = StatusState.getFeature(Feature.worldEditor);

  const itemAssetType = ProjectState.project.assetTypes[entry.path];
  const canUseAssetRenderer = itemAssetType === assetTypes.fxworld
    ? worldEditorEnabled
    : true;

  if (itemAssetType && canUseAssetRenderer) {
    const AssetRenderer = ENABLED_ASSET_RENDERERS[itemAssetType];
    if (AssetRenderer) {
      return (
        <AssetRenderer
          key={entry.path}
          {...props}
        />
      );
    }
  }

  if (entry.isFile) {
    return (
      <File
        key={entry.path}
        {...props}
      />
    );
  }

  if (entry.isDirectory) {
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

  if (entry.name === 'fxproject.json') {
    return false;
  }

  if (isAssetMetaFile(entry.name)) {
    return false;
  }

  return true;
};

export const ProjectExplorer = observer(function ProjectExplorer() {
  const project = ProjectState.project;

  const handleDirectoryCreate = React.useCallback((directoryName: string) => {
    ProjectState.closeDirectoryCreator();

    if (directoryName) {
      sendApiMessage(projectApi.createDirectory, {
        directoryPath: project.path,
        directoryName,
      });
    }
  }, [project.path]);

  const handleOpenResourceCreator = React.useCallback(() => {
    ProjectState.setResourceCreatorDir(project.path);
    ProjectState.openResourceCreator();
  }, [project.path]);

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
        onClick: ProjectState.openDirectoryCreator,
      },
    ];
  }, [handleOpenResourceCreator]);

  return (
    <ProjectExplorerContextProvider>
      <ContextMenu
        className={s.root}
        items={contextItems}
      >
        {ProjectState.directoryCreatorOpen && (
          <DirectoryCreator
            className={s.creator}
            onCreate={handleDirectoryCreate}
          />
        )}
        <DndProvider backend={HTML5Backend}>
          {nodes}

          {!nodes.length && (
            <>
              <div className={s.empty}>
                <WiWindy />

                <div>
                  Looks pretty empty here :(
                  <br />
                  Start by creating something new
                  <br />
                  or import existing stuff!
                </div>
              </div>
              <div className={s['quick-access']}>
                <button onClick={() => ProjectState.openResourceCreator()}>
                  {newResourceIcon}
                  New resource
                </button>

                <button onClick={() => ProjectState.openImporter()}>
                  {importAssetIcon}
                  Import asset
                </button>
              </div>
            </>
          )}
        </DndProvider>
      </ContextMenu>
    </ProjectExplorerContextProvider>
  );
});
