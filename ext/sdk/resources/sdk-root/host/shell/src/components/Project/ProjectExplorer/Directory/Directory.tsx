import React from 'react';
import classnames from 'classnames';
import { BsFolder, BsFolderFill, BsPuzzle } from 'react-icons/bs';
import { ContextMenu } from '../../../controls/ContextMenu/ContextMenu';
import { DirectoryDeleteConfirmation } from './DirectoryDeleteConfirmation/DirectoryDeleteConfirmation';
import { useDirectoryContextMenu } from './Directory.hooks';
import { ProjectItemProps } from '../ProjectExplorer.item';
import { FilesystemEntry, MoveEntryRequest, Project } from '../../../../sdkApi/api.types';
import { useExpandablePath, useItem, useItemDragAndDrop } from '../ProjectExplorer.hooks';
import s from './Directory.module.scss';
import { DropTargetMonitor, useDrop } from 'react-dnd';
import { EntryMoveItem, projectExplorerItemType } from '../ProjectExplorer.itemTypes';
import { sendApiMessage } from '../../../../utils/api';
import { projectApi } from '../../../../sdkApi/events';


const getDirectoryIcon = (entry: FilesystemEntry, open: boolean, project: Project) => {
  if (entry.meta.assetMeta) {
    return <BsPuzzle />;
  }

  return open
    ? <BsFolder />
    : <BsFolderFill />;
};


export interface DirectoryProps extends ProjectItemProps {
  icon?: React.ReactNode,
}

export const Directory = React.memo((props: DirectoryProps) => {
  const { entry, project, pathsMap } = props;
  const { icon } = props;

  const { expanded, toggleExpanded } = useExpandablePath(entry.path);

  const directoryChildren = pathsMap[entry.path] || [];

  const {
    directoryContextMenuItems,
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  } = useDirectoryContextMenu(entry.path, project, directoryChildren.length);

  const { contextMenuItems, renderItemControls, renderItemChildren, options } = useItem(props);

  const contextItems = [
    ...directoryContextMenuItems,
    ...contextMenuItems,
  ];
  const nodes = renderItemChildren();
  const iconNode = icon || getDirectoryIcon(entry, expanded, project);

  /**
   * Drag'n'Drop functionality
   */
  const { isDragging, isDropping, dragRef, dropRef } = useItemDragAndDrop(entry, projectExplorerItemType.FOLDER, [
    projectExplorerItemType.FILE, projectExplorerItemType.FOLDER
  ]);

  const rootClassName = classnames(s.root, {
    [s.dropping]: isDropping,
  })

  const nameClassName = classnames(s.name, {
    [s.dragging]: isDragging,
  });

  return (
    <div className={rootClassName} ref={dropRef}>
      <ContextMenu
        className={nameClassName}
        items={contextItems}
        onClick={toggleExpanded}
        activeClassName={s.active}
        ref={dragRef}
      >
        {iconNode}
        {entry.name}
      </ContextMenu>

      {expanded && (
        <div className={s.children}>
          {renderItemControls()}

          {nodes}
        </div>
      )}

      {deleteConfirmationOpen && (
        <DirectoryDeleteConfirmation
          path={entry.path}
          onClose={closeDeleteConfirmation}
          onDelete={deleteDirectory}
        />
      )}
    </div>
  );
});
