import * as React from 'react';
import { newDirectoryIcon, newFileIcon } from '../../../constants/icons';
import { projectApi } from '../../../sdkApi/events';
import { sendApiMessage } from '../../../utils/api';
import { useOpenFlag } from '../../../utils/hooks';
import { ContextMenuItem } from '../../controls/ContextMenu/ContextMenu';
import { VisibilityFilter } from '../../Explorer/Explorer';
import { DirectoryCreator } from './Directory/DirectoryCreator/DirectoryCreator';
import { FileCreator } from './File/FileCreator/FileCreator';
import { ProjectExplorerContext } from './ProjectExplorer.context';
import { ProjectItemProps, renderChildren } from './ProjectExplorer.item';
import { ProjectExplorerItemContext } from './ProjectExplorer.itemContext';

export interface UseExpandedPathHook {
  expanded: boolean,
  toggleExpanded: () => void,
  forceExpanded: () => void,
  forceCollapsed: () => void,
}

export const useExpandablePath = (path: string, expandedByDefault: boolean = true): UseExpandedPathHook => {
  const { pathsState, setPathState } = React.useContext(ProjectExplorerContext);

  const expanded = path in pathsState
    ? pathsState[path]
    : expandedByDefault;

  const toggleExpanded = React.useCallback(() => {
    setPathState(path, !expanded);
  }, [path, expanded, setPathState]);

  const forceExpanded = React.useCallback(() => {
    setPathState(path, true);
  }, [path, setPathState]);

  const forceCollapsed = React.useCallback(() => {
    setPathState(path, false);
  }, [path, setPathState]);

  return {
    expanded,
    toggleExpanded,
    forceExpanded,
    forceCollapsed,
  };
};


export interface UseItemHook {
  renderItemControls: () => React.ReactNode,
  renderItemChildren: (overrideVisibilityFilter?: VisibilityFilter) => React.ReactNode,
  contextMenuItems: ContextMenuItem[],
}

export const useItem = (item: ProjectItemProps): UseItemHook => {
  const { setPathState } = React.useContext(ProjectExplorerContext);
  const options = React.useContext(ProjectExplorerItemContext);

  const [directoryCreatorOpen, openDirectoryCreator, closeDirectoryCreator] = useOpenFlag(false);
  const handleDirectoryCreate = React.useCallback((directoryName: string) => {
    closeDirectoryCreator();

    if (directoryName) {
      sendApiMessage(projectApi.createDirectory, {
        directoryPath: item.entry.path,
        directoryName,
      });
    }
  }, [item, closeDirectoryCreator]);

  const [fileCreatorOpen, openFileCreator, closeFileCreator] = useOpenFlag(false);
  const handleFileCreate = React.useCallback((fileName: string) => {
    closeFileCreator();

    if (fileName) {
      sendApiMessage(projectApi.createFile, {
        filePath: item.entry.path,
        fileName,
      });
    }
  }, [item, closeFileCreator]);

  const renderItemControls = React.useCallback(() => {
    return (
      <>
        {directoryCreatorOpen && (
          <DirectoryCreator
            className={item.creatorClassName}
            onCreate={handleDirectoryCreate}
          />
        )}
        {fileCreatorOpen && (
          <FileCreator
            className={item.creatorClassName}
            onCreate={handleFileCreate}
          />
        )}
      </>
    );
  }, [item, directoryCreatorOpen, handleDirectoryCreate]);

  const renderItemChildren = React.useCallback((overrideVisibilityFilter?: VisibilityFilter) => {
    return renderChildren(item.entry, item, overrideVisibilityFilter || options.visibilityFilter);
  }, [item, options.visibilityFilter]);

  const contextMenuItems: ContextMenuItem[] = React.useMemo(() => [
    {
      id: 'new-directory',
      text: 'New directory',
      icon: newDirectoryIcon,
      disabled: options.disableDirectoryCreate,
      onClick: () => {
        setPathState(item.entry.path, true);
        openDirectoryCreator();
      },
    },
    {
      id: 'new-file',
      text: 'New file',
      icon: newFileIcon,
      disabled: options.disableFileCreate,
      onClick: () => {
        setPathState(item.entry.path, true);
        openFileCreator();
      },
    },
  ], [item, setPathState, openFileCreator]);

  return {
    renderItemControls,
    renderItemChildren,
    contextMenuItems,
  };
};
