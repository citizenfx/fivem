import React from 'react';
import { ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, newResourceIcon, renameIcon } from 'constants/icons';
import { useOpenFlag } from 'utils/hooks';
import { ProjectExplorerItemContext } from '../item.context';
import { ProjectState } from 'store/ProjectState';
import { FilesystemEntry } from 'shared/api.types';
import { Explorer } from 'components/Explorer/Explorer';
import { getRelativePath } from 'components/Explorer/Explorer.utils';

export const useDirectoryContextMenu = (entry: FilesystemEntry, childrenLength: number, optionsOverride: Partial<ProjectExplorerItemContext> = {}) => {
  const { disableDirectoryDelete, disableDirectoryRename, disableAssetCreate } = {
    ...React.useContext(ProjectExplorerItemContext),
    ...optionsOverride,
  };

  const [directoryRenameOpen, openDirectoryRename, closeDirectoryRename] = useOpenFlag(false);

  const handleDirectoryDelete = React.useCallback(() => {
    if (childrenLength > 0) {
      const directoryRelativePath = getRelativePath(ProjectState.project.path || '', entry.path);

      return ProjectState.project!.deleteEntryConfirmFirst(entry.path, `Delete directory "${directoryRelativePath}"?`, () => (
        <>
          <div className="modal-label">
            Directory structure:
          </div>

          <Explorer
            baseEntry={entry}
            pathsMap={ProjectState.project.fs}
          />
        </>
      ));
    } else {
      return ProjectState.project.deleteEntry(entry.path);
    }
  }, [entry, childrenLength]);

  const handleCreateResource = React.useCallback(() => {
    ProjectState.openResourceCreator(entry.path);
  }, [entry.path]);

  const directoryContextMenuItems: ContextMenuItemsCollection = React.useMemo(() => {
    return [
      {
        id: 'delete-directory',
        text: 'Delete directory',
        icon: deleteIcon,
        disabled: disableDirectoryDelete,
        onClick: handleDirectoryDelete,
      },
      {
        id: 'rename-directory',
        text: 'Rename directory',
        icon: renameIcon,
        disabled: disableDirectoryRename,
        onClick: openDirectoryRename,
      },
      ContextMenuItemSeparator,
      {
        id: 'new-resource',
        text: 'New resource',
        icon: newResourceIcon,
        disabled: disableAssetCreate,
        onClick: handleCreateResource,
      },
    ];
  }, [handleDirectoryDelete, handleCreateResource, openDirectoryRename, disableDirectoryDelete, disableAssetCreate, disableDirectoryRename]);

  return {
    directoryContextMenuItems,
    directoryRenameOpen,
    closeDirectoryRename,
  };
};
