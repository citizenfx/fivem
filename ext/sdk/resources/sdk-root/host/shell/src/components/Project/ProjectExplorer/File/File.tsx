import * as React from 'react';
import classnames from 'classnames';
import { ProjectContext } from '../../../../contexts/ProjectContext';
import { ProjectItemProps } from '../ProjectExplorer.item';
import { getFileIcon } from './File.utils';
import { ProjectExplorerItemContext } from '../ProjectExplorer.itemContext';
import { ContextMenu, ContextMenuItemsCollection } from '../../../controls/ContextMenu/ContextMenu';
import { deleteIcon, renameIcon } from '../../../../constants/icons';
import { useOpenFlag } from '../../../../utils/hooks';
import { FileRenamer } from './FileRenamer/FileRenamer';
import { FileDeleter } from './FileDeleter/FileDeleter';
import s from './File.module.scss';
import { useDrag } from 'react-dnd';
import { projectExplorerItemType } from '../ProjectExplorer.itemTypes';
import { sendApiMessage } from '../../../../utils/api';
import { projectApi } from '../../../../sdkApi/events';
import { MoveEntryRequest } from '../../../../sdkApi/api.types';


export const File = React.memo((props: ProjectItemProps) => {
  const { entry } = props;

  const { openFile } = React.useContext(ProjectContext);
  const options = React.useContext(ProjectExplorerItemContext);

  const [fileRenamerOpen, openFileRenamer, closeFileRenamer] = useOpenFlag(false);
  const [fileDeleterOpen, openFileDeleter, closeFileDeleter] = useOpenFlag(false);

  const handleClick = React.useCallback(() => {
    if (!options.disableFileOpen) {
      openFile(entry);
    }
  }, [entry, options, openFile]);

  const contextMenuItems: ContextMenuItemsCollection = React.useMemo(() => [
    {
      id: 'delete-file',
      icon: deleteIcon,
      text: 'Delete file',
      disabled: options.disableFileDelete,
      onClick: openFileDeleter,
    },
    {
      id: 'rename-file',
      icon: renameIcon,
      text: 'Rename file',
      disabled: options.disableFileRename,
      onClick: openFileRenamer,
    },
  ], [options, openFileDeleter, openFileRenamer]);

  const icon = getFileIcon(entry);

  const [{ isDragging }, dragRef] = useDrag({
    item: {
      entry,
      type: projectExplorerItemType.FILE,
    },
    canDrag: () => !options.disableEntryMove,
    end: (item, monitor) => {
      const dropTarget = monitor.getDropResult();

      const sourcePath = item?.entry.path;
      const targetPath = dropTarget?.entry.path;

      if (sourcePath && targetPath) {
        const moveEntryRequest: MoveEntryRequest = {
          sourcePath,
          targetPath,
        };

        sendApiMessage(projectApi.moveEntry, moveEntryRequest);
      }
    },
    collect: (monitor) => ({
      isDragging: monitor.isDragging(),
    }),
  });

  const rootClassName = classnames(s.root, {
    [s.dragging]: isDragging,
  });

  return (
    <>
      <ContextMenu
        ref={dragRef}
        items={contextMenuItems}
        className={rootClassName}
        onClick={handleClick}
      >
        {icon}
        {entry.name}
      </ContextMenu>

      {fileRenamerOpen && (
        <FileRenamer entry={entry} onClose={closeFileRenamer} />
      )}

      {fileDeleterOpen && (
        <FileDeleter entry={entry} onClose={closeFileDeleter} />
      )}
    </>
  );
});
