import React from 'react';
import { observer } from 'mobx-react-lite';
import { DndProvider } from 'react-dnd';
import { HTML5Backend } from 'react-dnd-html5-backend';
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
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import { isAssetMetaFile } from 'utils/project';
import { ProjectState } from 'store/ProjectState';
import s from './ProjectExplorer.module.scss';


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
        <ScrollContainer>
          {ProjectState.directoryCreatorOpen && (
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
