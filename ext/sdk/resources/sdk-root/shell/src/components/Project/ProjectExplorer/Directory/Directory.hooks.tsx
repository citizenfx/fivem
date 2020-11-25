import React from 'react';
import { ContextMenuItem } from 'components/controls/ContextMenu/ContextMenu';
import { deleteIcon, newResourceIcon } from 'constants/icons';
import { ProjectContext } from 'contexts/ProjectContext';
import { Project } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useOpenFlag } from 'utils/hooks';
import { ProjectExplorerItemContext } from '../ProjectExplorer.itemContext';


export const useDirectoryContextMenu = (path: string, project: Project, childrenLength: number) => {
  const { setAssetCreatorDir, openAssetCreator } = React.useContext(ProjectContext);
  const { disableDirectoryDelete, disableAssetCreate } = React.useContext(ProjectExplorerItemContext);

  const [deleteConfirmationOpen, openDeleteConfirmation, closeDeleteConfirmation] = useOpenFlag(false);

  const deleteDirectory = React.useCallback(() => {
    sendApiMessage(projectApi.deleteDirectory, {
      projectPath: project.path,
      directoryPath: path,
    });
  }, [project, path]);

  const handleDirectoryDelete = React.useCallback(() => {
    if (childrenLength > 0) {
      openDeleteConfirmation();
    } else {
      deleteDirectory();
    }
  }, [deleteDirectory, openDeleteConfirmation, childrenLength]);

  const handleCreateResource = React.useCallback(() => {
    setAssetCreatorDir(path);
    openAssetCreator();
  }, [path, setAssetCreatorDir, openAssetCreator]);

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
        id: 'new-asset',
        text: 'New asset',
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
