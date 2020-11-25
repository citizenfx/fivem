import * as React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Modal } from 'components/Modal/Modal';
import { FilesystemEntry } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';


export interface FileDeleterProps {
  entry: FilesystemEntry,
  onClose: () => void,
}

export const FileDeleter = React.memo(function FileDeleter(props: FileDeleterProps) {
  const { entry, onClose } = props;

  const handleDelete = React.useCallback(() => {
    sendApiMessage(projectApi.deleteFile, {
      filePath: entry.path,
    });
  }, [entry]);

  return (
    <Modal onClose={onClose}>
      <div className="modal-header">
        Delete "{entry.name}" file?
      </div>

      <div className="modal-actions">
        <Button
          text="Delete file"
          onClick={handleDelete}
        />
        <Button
          theme="primary"
          text="Cancel"
          onClick={onClose}
          autofocus
        />
      </div>
    </Modal>
  );
});
