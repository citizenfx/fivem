import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Explorer } from 'components/Explorer/Explorer';
import { Modal } from 'components/Modal/Modal';
import { ProjectContext } from 'contexts/ProjectContext';
import { getRelativePath } from 'components/Explorer/Explorer.utils';
import { FilesystemEntry } from 'shared/api.types';

export interface DirectoryDeleteConfirmationProps {
  entry: FilesystemEntry,
  onClose: () => void,
  onDelete: () => void,
}

export const DirectoryDeleteConfirmation = React.memo(function DirectoryDeleteConfirmation({ entry, onClose, onDelete }: DirectoryDeleteConfirmationProps) {
  const { project } = React.useContext(ProjectContext);
  const directoryRelativePath = getRelativePath(project?.path || '', entry.path);

  return (
    <Modal onClose={onClose}>
      <div className="modal-header">
        Delete directory "{directoryRelativePath}"?
      </div>

      <div className="modal-label">
        Directory has following structure:
      </div>

      <Explorer
        baseEntry={entry}
        pathsMap={project?.fs}
      />

      <div className="modal-actions">
        <Button
          text="Delete directory"
          onClick={onDelete}
          tabIndex={1}
        />
        <Button
          theme="primary"
          text="Cancel"
          onClick={onClose}
          autofocus
          tabIndex={0}
        />
      </div>
    </Modal>
  );
});
