import React from 'react';
import { deleteIcon, newDirectoryIcon, newResourceIcon } from '../../../../constants/icons';
import { ProjectContext } from '../../../../contexts/ProjectContext';
import { Project } from '../../../../sdkApi/api.types';
import { projectApi } from '../../../../sdkApi/events';
import { sendApiMessage } from '../../../../utils/api';
import { useOpenFlag } from '../../../../utils/hooks';
import { ContextMenuItem } from '../../../controls/ContextMenu/ContextMenu';
import { DirectoryContext } from './Directory.context';


export const useDirectoryContextMenu = (path: string, project: Project, setOpen: () => void, childrenLength: number) => {
  const { setAssetCreatorDir, openAssetCreator } = React.useContext(ProjectContext);
  const { forbidDeleteDirectory, forbidCreateDirectory, forbidCreateResource } = React.useContext(DirectoryContext);

  const [deleteConfirmationOpen, openDeleteConfirmation, closeDeleteConfirmation] = useOpenFlag(false);

  const [creatorOpen, openCreator, closeCreator] = useOpenFlag(false);
  const handleDirectoryCreate = React.useCallback((directoryName: string) => {
    closeCreator();

    if (directoryName) {
      sendApiMessage(projectApi.createDirectory, {
        projectPath: project.path,
        directoryPath: path,
        directoryName,
      });
    }
  }, [path, project, closeCreator]);

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

  const ctxItems: ContextMenuItem[] = React.useMemo(() => {
    return [
      {
        id: 'add-child-directory',
        text: 'Add child directory',
        icon: newDirectoryIcon,
        disabled: forbidCreateDirectory,
        onClick: () => {
          setOpen();
          openCreator();
        },
      },
      {
        id: 'add-child-directory',
        text: 'Delete directory',
        icon: deleteIcon,
        disabled: forbidDeleteDirectory,
        onClick: handleDirectoryDelete,
      },
      {
        id: 'add-resource',
        text: 'Create resource',
        icon: newResourceIcon,
        disabled: forbidCreateResource,
        onClick: handleCreateResource,
      },
    ];
  }, [setOpen, openCreator, handleDirectoryDelete, handleCreateResource, forbidDeleteDirectory, forbidCreateDirectory, forbidCreateResource]);

  return {
    ctxItems,
    creatorOpen,
    handleDirectoryCreate,
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  };
};
