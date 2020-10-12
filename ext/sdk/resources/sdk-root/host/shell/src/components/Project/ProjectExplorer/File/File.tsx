import * as React from 'react';
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
      onClick: openFileDeleter,
    },
    {
      id: 'rename-file',
      icon: renameIcon,
      text: 'Rename file',
      onClick: openFileRenamer,
    },
  ], [options, openFileDeleter, openFileRenamer]);

  const icon = getFileIcon(entry);

  return (
    <>
      <ContextMenu
        items={contextMenuItems}
        className={s.root}
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
