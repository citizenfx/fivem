import React from 'react';
import classnames from 'classnames';
import { BsFolder, BsFolderFill, BsPuzzle } from 'react-icons/bs';
import { ContextMenu } from '../../../controls/ContextMenu/ContextMenu';
import { DirectoryDeleteConfirmation } from './DirectoryDeleteConfirmation/DirectoryDeleteConfirmation';
import { useDirectoryContextMenu } from './Directory.hooks';
import { ProjectItemProps } from '../ProjectExplorer.item';
import { FilesystemEntry, Project } from '../../../../sdkApi/api.types';
import { useExpandablePath, useItem } from '../ProjectExplorer.hooks';
import s from './Directory.module.scss';
import { useDrop } from 'react-dnd';
import { projectExplorerItemType } from '../ProjectExplorer.itemTypes';


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

  const { contextMenuItems, renderItemControls, renderItemChildren } = useItem(props);

  const contextItems = [
    ...directoryContextMenuItems,
    ...contextMenuItems,
  ];
  const nodes = renderItemChildren();
  const iconNode = icon || getDirectoryIcon(entry, expanded, project);

  const [{ canDrop, isOver }, dropRef] = useDrop({
    accept: [projectExplorerItemType.FILE, projectExplorerItemType.FOLDER],
    drop: () => ({ entry }),
    collect: (monitor) => ({
      isOver: monitor.isOver(),
      canDrop: monitor.canDrop(),
    }),
  });

  const nameClassName = classnames(s.name, {
    [s['drop-ready']]: canDrop && isOver,
  });

  return (
    <div className={s.root}>
      <ContextMenu
        ref={dropRef}
        className={nameClassName}
        items={contextItems}
        onClick={toggleExpanded}
        activeClassName={s.active}
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
