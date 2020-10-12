import * as React from 'react';
import { FilesystemEntry } from '../../../../../sdkApi/api.types';
import { projectApi } from '../../../../../sdkApi/events';
import { sendApiMessage } from '../../../../../utils/api';
import { Button } from '../../../../controls/Button/Button';
import { Modal } from '../../../../Modal/Modal';


export interface FileDeleterProps {
  entry: FilesystemEntry,
  onClose: () => void,
}

export const FileDeleter = React.memo((props: FileDeleterProps) => {
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
