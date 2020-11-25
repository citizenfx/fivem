import * as React from 'react';
import { DropTargetMonitor, useDrag, useDrop } from 'react-dnd';
import { ContextMenuItem, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { VisibilityFilter } from 'components/Explorer/Explorer';
import { newDirectoryIcon, newFileIcon } from 'constants/icons';
import { FilesystemEntry, MoveEntryRequest } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useOpenFlag } from 'utils/hooks';
import { DirectoryCreator } from './Directory/DirectoryCreator/DirectoryCreator';
import { FileCreator } from './File/FileCreator/FileCreator';
import { EntryRelocateOperation, ProjectExplorerContext } from './ProjectExplorer.context';
import { ProjectItemProps, renderChildren } from './ProjectExplorer.item';
import { ProjectExplorerItemContext } from './ProjectExplorer.itemContext';
import { EntryMoveItem } from './ProjectExplorer.itemTypes';

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
  options: ProjectExplorerItemContext,
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

  const renderItemControls = React.useCallback(function $RenderItemControls() {
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
  }, [item, directoryCreatorOpen, fileCreatorOpen, handleDirectoryCreate, handleFileCreate]);

  const renderItemChildren = React.useCallback(function $RenderItemChildren(overrideVisibilityFilter?: VisibilityFilter) {
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
    options,
  };
};

export const useItemDrag = (entry: FilesystemEntry, type: string) => {
  const options = React.useContext(ProjectExplorerItemContext);

  const [{ isDragging }, dragRef] = useDrag({
    item: { entry, type },
    canDrag: () => !options.disableEntryMove,
    collect: (monitor) => ({
      isDragging: monitor.isDragging(),
    }),
  });

  return {
    isDragging,
    dragRef,
  };
};

export const useItemDrop = (entry: FilesystemEntry, accept: string | string[]) => {
  const options = React.useContext(ProjectExplorerItemContext);

  const [{ isDropping }, dropRef] = useDrop({
    accept,
    drop: (item: EntryMoveItem, monitor: DropTargetMonitor) => {
      // Only allowing the closest drop target to accept drop
      if (monitor.didDrop()) {
        return;
      }

      const sourcePath = item?.entry?.path;
      const targetPath = entry.path;

      if (sourcePath && targetPath) {
        const moveEntryRequest: MoveEntryRequest = {
          sourcePath,
          targetPath,
        };

        sendApiMessage(projectApi.moveEntry, moveEntryRequest);
      }
    },
    canDrop: () => !options.disableEntryMove,
    collect: (monitor) => ({
      isDropping: monitor.isOver({ shallow: true }) && monitor.canDrop(),
    }),
  });

  return {
    isDropping,
    dropRef,
  };
};

export const useItemDragAndDrop = (entry: FilesystemEntry, type: string, accept: string | string[]) => {
  const { isDragging, dragRef } = useItemDrag(entry, type);
  const { isDropping, dropRef } = useItemDrop(entry, accept);

  return {
    isDragging,
    isDropping,
    dragRef,
    dropRef,
  };
};

export const useItemRelocateSourceContextMenu = (entry: FilesystemEntry) => {
  const { setRelocationContext } = React.useContext(ProjectExplorerContext);
  const { disableEntryMove } = React.useContext(ProjectExplorerItemContext);

  return React.useMemo((): ContextMenuItemsCollection => [
    {
      id: 'relocate-copy',
      text: 'Copy',
      disabled: disableEntryMove,
      onClick: () => setRelocationContext(entry, EntryRelocateOperation.Copy),
    },
    {
      id: 'relocate-move',
      text: 'Cut',
      disabled: disableEntryMove,
      onClick: () => setRelocationContext(entry, EntryRelocateOperation.Move),
    },
  ], [entry, setRelocationContext, disableEntryMove]);
};

export const useItemRelocateTargetContextMenu = (entry: FilesystemEntry) => {
  const { applyRelocation, relocateSourceEntry } = React.useContext(ProjectExplorerContext);
  const { disableEntryMove } = React.useContext(ProjectExplorerItemContext);

  return React.useMemo((): ContextMenuItemsCollection => [
    {
      id: 'relocate-paste',
      text: relocateSourceEntry
        ? `Paste "${relocateSourceEntry.name}"`
        : 'Paste',
      disabled: disableEntryMove || !relocateSourceEntry,
      onClick: () => applyRelocation(entry),
    },
  ], [entry, applyRelocation, relocateSourceEntry, disableEntryMove]);
};
