import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { BsFolder, BsFolderFill, BsPuzzle } from 'react-icons/bs';
import { useDirectoryContextMenu } from './Directory.hooks';
import { ProjectItemProps } from '../item';
import { useExpandablePath, useItem, useItemDragAndDrop, useItemRelocateSourceContextMenu, useItemRelocateTargetContextMenu } from '../ProjectExplorer.hooks';
import { projectExplorerItemType } from '../item.types';
import { FilesystemEntry } from 'shared/api.types';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'fxdk/ui/controls/ContextMenu/ContextMenu';
import { itemsStyles } from '../item.styles';
import { NativeTypes } from 'react-dnd-html5-backend';
import { ProjectExplorerItemContext, ProjectExplorerItemContextProvider } from '../item.context';
import { DirectoryRenamer } from './DirectoryRenamer/DirectoryRenamer';


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

export const Directory = observer(function Directory(props: DirectoryProps) {
  const { entry, pathsMap } = props;
  const { icon } = props;

  const { expanded, toggleExpanded } = useExpandablePath(entry.path, !props.childrenCollapsed);

  const directoryChildren = pathsMap[entry.path] || [];

  const assetMetaFlags = entry.meta?.assetMeta?.flags;
  const itemContext = React.useMemo(() => {
    if (!assetMetaFlags) {
      return {};
    }

    const ctx: Partial<ProjectExplorerItemContext> = {};

    if (assetMetaFlags.readOnly) {
      ctx.disableAssetCreate = true;
      ctx.disableAssetDelete = true;
      ctx.disableAssetRename = true;
      ctx.disableDirectoryCreate = true;
      ctx.disableDirectoryDelete = true;
      ctx.disableDirectoryRename = true;
      ctx.disableEntryMove = true;
      ctx.disableFileCreate = true;
      ctx.disableFileDelete = true;
      ctx.disableFileOpen = true;
      ctx.disableFileRename = true;
    }

    return ctx;
  }, [assetMetaFlags]);

  const {
    directoryContextMenuItems,
    closeDirectoryRename,
    directoryRenameOpen,
  } = useDirectoryContextMenu(entry, directoryChildren.length, itemContext);

  const { contextMenuItems, requiredContextMenuItems, renderItemControls, renderItemChildren } = useItem(props, itemContext);

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
    projectExplorerItemType.FILE,
    projectExplorerItemType.FOLDER,
    projectExplorerItemType.ASSET,
    NativeTypes.FILE,
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
        {assetMetaFlags?.readOnly && (
          <div className={itemsStyles.itemStatus}>
            <div className={itemsStyles.itemStatusEntry}>
              readonly
            </div>
          </div>
        )}
      </ContextMenu>

      {expanded && (
        <div className={itemsStyles.children}>
          {renderItemControls()}

          <ProjectExplorerItemContextProvider options={itemContext}>
            {nodes}
          </ProjectExplorerItemContextProvider>
        </div>
      )}

      {directoryRenameOpen && (
        <DirectoryRenamer
          entry={entry}
          onClose={closeDirectoryRename}
        />
      )}
    </div>
  );
});
