import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Modal } from 'components/Modal/Modal';
import { AssetDeleteRequest } from 'shared/api.requests';
import { assetApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import s from './ResourceDeleter.module.scss';
import { ProjectContext } from 'contexts/ProjectContext';


export interface ResourceDeleterProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const ResourceDeleter = React.memo(function ResourceDeleter({ name, path, onClose }: ResourceDeleterProps) {
  const { addPendingDirectoryDeletion } = React.useContext(ProjectContext);

  const handleDeleteResource = React.useCallback(() => {
    addPendingDirectoryDeletion(path);

    const request: AssetDeleteRequest = {
      assetPath: path,
    };

    sendApiMessage(assetApi.delete, request);
    onClose();
  }, [path, onClose, addPendingDirectoryDeletion]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Delete "{name}" Resource?
        </div>

        <div className={s.disclaimer}>
          This action can not be reverted. Are you sure?
        </div>

        <div className="modal-actions">
          <Button
            text="Delete resource"
            onClick={handleDeleteResource}
          />

          <Button
            theme="primary"
            text="Cancel"
            onClick={onClose}
            autofocus
          />
        </div>
      </div>
    </Modal>
  );
});
