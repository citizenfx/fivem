import * as React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { useOpenFlag } from 'utils/hooks';
import { ContextMenu, ContextMenuItemsCollection, ContextMenuItemSeparator } from 'fxdk/ui/controls/ContextMenu/ContextMenu';
import { deleteIcon, renameIcon } from 'constants/icons';
import { getFileIcon } from './File.utils';
import { FileRenamer } from './FileRenamer/FileRenamer';
import { ProjectExplorerItemContext } from '../item.context';
import { projectExplorerItemType } from '../item.types';
import { ProjectItemProps } from '../item';
import { useItemDrag, useItemRelocateSourceContextMenu } from '../ProjectExplorer.hooks';
import { itemsStyles } from '../item.styles';
import { BsBoxArrowUpRight } from 'react-icons/bs';
import { ProjectState, useFsTreeEntryAccessor } from 'store/ProjectState';


export const File = observer(function File(props: ProjectItemProps) {
  const { entry } = props;

  const rootRef = React.useRef<HTMLDivElement>(null);

  const [selected, setSelected] = React.useState(false);
  const handleSetSelected = React.useCallback((selected: boolean) => {
    setSelected(selected);

    if (selected && rootRef.current) {
      ((rootRef.current as any).scrollIntoViewIfNeeded as HTMLDivElement['scrollIntoView'])({
        block: 'center',
      });
    }
  }, []);

  useFsTreeEntryAccessor(entry.path, () => ({ setSelected: handleSetSelected }));

  const options = React.useContext(ProjectExplorerItemContext);

  const relocateSourceContextMenu = useItemRelocateSourceContextMenu(entry);

  const [fileRenamerOpen, openFileRenamer, closeFileRenamer] = useOpenFlag(false);

  const handleClick = React.useCallback(() => {
    if (!options.disableFileOpen) {
      ProjectState.openFile(entry);
    }
  }, [entry, options]);

  const handleDoubleClick = React.useCallback(() => {
    if (!options.disableFileOpen) {
      ProjectState.openFile(entry, true);
    }
  }, [entry, options]);

  const contextMenuItems: ContextMenuItemsCollection = React.useMemo(() => [
    ...relocateSourceContextMenu,
    ContextMenuItemSeparator,
    {
      id: 'delete-file',
      icon: deleteIcon,
      text: 'Delete file',
      disabled: options.disableFileDelete,
      onClick: () => ProjectState.project!.deleteEntry(entry.path),
    },
    {
      id: 'rename-file',
      icon: renameIcon,
      text: 'Rename file',
      disabled: options.disableFileRename,
      onClick: openFileRenamer,
    },
    ContextMenuItemSeparator,
    {
      id: 'open-in-explorer',
      icon: <BsBoxArrowUpRight />,
      text: 'Open in Explorer',
      onClick: () => invokeNative('openFolderAndSelectFile', entry.path),
    },
  ], [entry, options, openFileRenamer, relocateSourceContextMenu]);

  const icon = getFileIcon(entry);

  const { isDragging, dragRef } = useItemDrag(entry, projectExplorerItemType.FILE);

  const rootClassName = classnames(itemsStyles.wrapper, {
    [itemsStyles.dragging]: isDragging,
    [itemsStyles.itemSelected]: selected,
  });

  return (
    <div ref={rootRef} className={rootClassName}>
      <ContextMenu
        ref={dragRef}
        items={contextMenuItems}
        className={itemsStyles.item}
        activeClassName={itemsStyles.itemActive}
        onClick={handleClick}
        onDoubleClick={handleDoubleClick}
      >
        <div className={itemsStyles.itemIcon}>
          {icon}
        </div>
        <div className={itemsStyles.itemTitle}>
          {entry.name}
        </div>
      </ContextMenu>

      {fileRenamerOpen && (
        <FileRenamer entry={entry} onClose={closeFileRenamer} />
      )}
    </div>
  );
});
