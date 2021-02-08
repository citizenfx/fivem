import React from 'react';
import classnames from 'classnames';
import { BsBoxArrowUpRight, BsFolder, BsFolderFill, BsPuzzle } from 'react-icons/bs';
import { DirectoryDeleteConfirmation } from './DirectoryDeleteConfirmation/DirectoryDeleteConfirmation';
import { useDirectoryContextMenu } from './Directory.hooks';
import { ProjectItemProps } from '../item';
import { useExpandablePath, useItem, useItemDragAndDrop, useItemRelocateSourceContextMenu, useItemRelocateTargetContextMenu } from '../ProjectExplorer.hooks';
import { projectExplorerItemType } from '../item.types';
import { FilesystemEntry } from 'shared/api.types';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'components/controls/ContextMenu/ContextMenu';
import { itemsStyles } from '../item.styles';


const getDirectoryIcon = (entry: FilesystemEntry, open: boolean) => {
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

export const Directory = React.memo(function Directory(props: DirectoryProps) {
  const { entry, project, pathsMap } = props;
  const { icon } = props;

  const { expanded, toggleExpanded } = useExpandablePath(entry.path, !props.childrenCollapsed);

  const directoryChildren = pathsMap[entry.path] || [];

  const {
    directoryContextMenuItems,
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  } = useDirectoryContextMenu(entry.path, project, directoryChildren.length);

  const { contextMenuItems, requiredContextMenuItems, renderItemControls, renderItemChildren } = useItem(props);

  const relocateSourceContextMenu = useItemRelocateSourceContextMenu(entry);
  const relocateTargetContextMenu = useItemRelocateTargetContextMenu(entry);

  const contextItems = React.useMemo((): ContextMenuItemsCollection => [
    ...relocateSourceContextMenu,
    ...relocateTargetContextMenu,
    ContextMenuItemSeparator,
    ...directoryContextMenuItems,
    ...contextMenuItems,
    ContextMenuItemSeparator,
    ...requiredContextMenuItems,
  ], [entry, relocateSourceContextMenu, relocateTargetContextMenu, directoryContextMenuItems, contextMenuItems, requiredContextMenuItems]);
  const nodes = renderItemChildren();
  const iconNode = icon || getDirectoryIcon(entry, expanded);

  /**
   * Drag'n'Drop functionality
   */
  const { isDragging, isDropping, dragRef, dropRef } = useItemDragAndDrop(entry, projectExplorerItemType.FOLDER, [
    projectExplorerItemType.FILE, projectExplorerItemType.FOLDER
  ]);

  const rootClassName = classnames(itemsStyles.wrapper, {
    [itemsStyles.dropping]: isDropping,
  })

  const itemClassName = classnames(itemsStyles.item, {
    [itemsStyles.dragging]: isDragging,
  });

  return (
    <div className={rootClassName} ref={dropRef}>
      <ContextMenu
        className={itemClassName}
        items={contextItems}
        onClick={toggleExpanded}
        activeClassName={itemsStyles.itemActive}
        ref={dragRef}
      >
        <div className={itemsStyles.itemIcon}>
          {iconNode}
        </div>
        <div className={itemsStyles.itemTitle}>
          {entry.name}
        </div>
      </ContextMenu>

      {expanded && (
        <div className={itemsStyles.children}>
          {renderItemControls()}

          {nodes}
        </div>
      )}

      {deleteConfirmationOpen && (
        <DirectoryDeleteConfirmation
          entry={entry}
          onClose={closeDeleteConfirmation}
          onDelete={deleteDirectory}
        />
      )}
    </div>
  );
});
