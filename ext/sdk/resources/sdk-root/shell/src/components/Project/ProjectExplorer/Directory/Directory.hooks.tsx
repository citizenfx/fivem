import React from 'react';
import { ContextMenuItem } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, newResourceIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';
import { ProjectData } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useOpenFlag } from 'utils/hooks';
import { ProjectExplorerItemContext } from '../item.context';


export const useDirectoryContextMenu = (path: string, project: ProjectData, childrenLength: number) => {
  const { setResourceCreatorDir: setAssetCreatorDir, openResourceCreator, addPendingDirectoryDeletion } = React.useContext(ProjectContext);
  const { disableDirectoryDelete, disableAssetCreate } = React.useContext(ProjectExplorerItemContext);

  const [deleteConfirmationOpen, openDeleteConfirmation, closeDeleteConfirmation] = useOpenFlag(false);

  const deleteDirectory = React.useCallback(() => {
    addPendingDirectoryDeletion(path);

    sendApiMessage(projectApi.deleteDirectory, {
      projectPath: project.path,
      directoryPath: path,
    });
  }, [project, path, addPendingDirectoryDeletion]);

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

  const directoryContextMenuItems: ContextMenuItem[] = React.useMemo(() => {
    return [
      {
        id: 'delete-directory',
        text: 'Delete directory',
        icon: deleteIcon,
        disabled: disableDirectoryDelete,
        onClick: handleDirectoryDelete,
      },
      {
        id: 'new-resource',
        text: 'New resource',
        icon: newResourceIcon,
        disabled: disableAssetCreate,
        onClick: handleCreateResource,
      },
    ];
  }, [handleDirectoryDelete, handleCreateResource, disableDirectoryDelete, disableAssetCreate]);

  return {
    directoryContextMenuItems,
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  };
};
