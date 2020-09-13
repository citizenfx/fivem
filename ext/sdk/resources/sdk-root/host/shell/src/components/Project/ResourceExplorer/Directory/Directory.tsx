import React from 'react';
import { BsFolder, BsFolderFill, BsPuzzle } from 'react-icons/bs';
import { useOpenFlag } from '../../../../utils/hooks';
import { ContextMenu } from '../../../controls/ContextMenu/ContextMenu';
import { DirectoryCreator } from './DirectoryCreator/DirectoryCreator';
import { DirectoryDeleteConfirmation } from './DirectoryDeleteConfirmation/DirectoryDeleteConfirmation';
import { useDirectoryContextMenu } from './Directory.hooks';
import { ProjectItemProps, renderChildren } from '../ResourceExplorer.item';
import { DirectoryContext } from './Directory.context';
import { FilesystemEntry, Project } from '../../../../sdkApi/api.types';
import s from './Directory.module.scss';


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
  visibilityFilter?: (entry: FilesystemEntry) => boolean,
  forbidCreateDirectory?: boolean,
  forbidDeleteDirectory?: boolean,
  forbidCreateResource?: boolean,
}

export const Directory = React.memo((props: DirectoryProps) => {
  const { entry, project, pathsMap, itemRenderer, creatorClassName } = props;
  const { icon } = props;
  const { visibilityFilter } = React.useContext(DirectoryContext);

  const directoryChildren = pathsMap[entry.path] || [];

  const [open, setOpen, _, toggleOpen] = useOpenFlag(true); // eslint-disable-line @typescript-eslint/no-unused-vars
  const {
    ctxItems,
    creatorOpen,
    handleDirectoryCreate,
    deleteConfirmationOpen,
    closeDeleteConfirmation,
    deleteDirectory,
  } = useDirectoryContextMenu(entry.path, project, setOpen, directoryChildren.length);

  const nodes = renderChildren(entry, props, visibilityFilter);

  const iconNode = icon || getDirectoryIcon(entry, open, project);

  return (
    <div className={s.root}>
      <ContextMenu
        className={s.name}
        items={ctxItems}
        onClick={toggleOpen}
        activeClassName={s.active}
      >
        {iconNode}
        {entry.name}
      </ContextMenu>

      {open && (
        <div className={s.children}>
          {creatorOpen && (
            <DirectoryCreator
              className={creatorClassName}
              onCreate={handleDirectoryCreate}
            />
          )}

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
