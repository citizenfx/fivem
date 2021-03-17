import React from 'react';
import { ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, newResourceIcon, renameIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useOpenFlag, useSendApiMessageCallback } from 'utils/hooks';
import { ProjectExplorerItemContext } from '../item.context';
import { DeleteDirectoryRequest, DeleteDirectoryResponse } from 'shared/api.requests';


export const useDeleteDirectoryApiCallbackMessage = (path: string) => {
  const { addPendingDirectoryDeletion } = React.useContext(ProjectContext);

  const deleteDirectory = useSendApiMessageCallback<DeleteDirectoryRequest, DeleteDirectoryResponse>(projectApi.deleteDirectory, (error, response) => {
    if (error) {
      return;
    }

    if (response === DeleteDirectoryResponse.FailedToRecycle) {
      if (window.confirm('Failed to recycle directory, delete it permanently?')) {
        addPendingDirectoryDeletion(path);

        sendApiMessage(projectApi.deleteDirectory, {
          directoryPath: path,
          hardDelete: true,
        } as DeleteDirectoryRequest);
      }
    }
  });

  return React.useCallback(() => {
    addPendingDirectoryDeletion(path);

    deleteDirectory({
      directoryPath: path,
    });
  }, [path, deleteDirectory, addPendingDirectoryDeletion]);
};

export const useDirectoryContextMenu = (path: string, childrenLength: number, optionsOverride: Partial<ProjectExplorerItemContext> = {}) => {
  const { setResourceCreatorDir: setAssetCreatorDir, openResourceCreator } = React.useContext(ProjectContext);
  const { disableDirectoryDelete, disableDirectoryRename, disableAssetCreate } = {
    ...React.useContext(ProjectExplorerItemContext),
    ...optionsOverride,
  };

  const [directoryRenameOpen, openDirectoryRename, closeDirectoryRename] = useOpenFlag(false);

  const [deleteConfirmationOpen, openDeleteConfirmation, closeDeleteConfirmation] = useOpenFlag(false);
  const deleteDirectory = useDeleteDirectoryApiCallbackMessage(path);

  const handleDirectoryDelete = React.useCallback(() => {
    if (childrenLength > 0) {
      openDeleteConfirmation();
    } else {
      deleteDirectory();
    }
  }, [deleteDirectory, openDeleteConfirmation, childrenLength]);

  const handleCreateResource = React.useCallback(() => {
    setAssetCreatorDir(path);
    openResourceCreator();
  }, [path, setAssetCreatorDir, openResourceCreator]);

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
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  };
};
