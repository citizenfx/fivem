import * as React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Modal } from 'components/Modal/Modal';
import { FilesystemEntry } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useSendApiMessageCallback } from 'utils/hooks';
import { DeleteFileRequest, DeleteFileResponse } from 'shared/api.requests';


export interface FileDeleterProps {
  entry: FilesystemEntry,
  onClose: () => void,
}

export const FileDeleter = React.memo(function FileDeleter(props: FileDeleterProps) {
  const { entry, onClose } = props;

  const deleteFile = useSendApiMessageCallback<DeleteFileRequest, DeleteFileResponse>(projectApi.deleteFile, (error, response) => {
    if (error) {
      return;
    }

    if (response === DeleteFileResponse.FailedToRecycle) {
      if (window.confirm('Failed to recycle file, delete it permanently?')) {
        sendApiMessage(projectApi.deleteFile, {
          filePath: entry.path,
          hardDelete: true,
        } as DeleteFileRequest);
      }
    }
  });

  const handleDelete = React.useCallback(() => {
    deleteFile({
      filePath: entry.path,
    });
  }, [entry, deleteFile]);

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
          autofocus
          theme="primary"
          text="Cancel"
          onClick={onClose}
        />
      </div>
    </Modal>
  );
});
