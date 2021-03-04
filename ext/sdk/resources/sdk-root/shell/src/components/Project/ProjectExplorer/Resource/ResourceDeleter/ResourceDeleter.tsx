import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Modal } from 'components/Modal/Modal';
import { AssetDeleteRequest, AssetDeleteResponse } from 'shared/api.requests';
import { assetApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import s from './ResourceDeleter.module.scss';
import { ProjectContext } from 'contexts/ProjectContext';
import { useSendApiMessageCallback } from 'utils/hooks';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';


export interface ResourceDeleterProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const ResourceDeleter = React.memo(function ResourceDeleter({ name, path, onClose }: ResourceDeleterProps) {
  const { addPendingDirectoryDeletion } = React.useContext(ProjectContext);
  const [recycle, setRecycle] = React.useState(true);
  const [deleteError, setDeleteError] = React.useState('');

  const deleteResource = useSendApiMessageCallback<AssetDeleteRequest, AssetDeleteResponse>(assetApi.delete, (error, response) => {
    if (error) {
      return setDeleteError(error);
    }

    if (response === AssetDeleteResponse.FailedToRecycle) {
      return setDeleteError('Failed to recycle, most likely due to disabled recycle bin, try again without recycle option set or enable recycle bin');
    }

    onClose();
  });

  const handleDeleteResource = React.useCallback(() => {
    setDeleteError('');

    addPendingDirectoryDeletion(path);

    const request: AssetDeleteRequest = {
      assetPath: path,
      hardDelete: !recycle,
    };

    deleteResource(request);

    onClose();
  }, [path, recycle, setDeleteError, deleteResource, addPendingDirectoryDeletion]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Delete "{name}" Resource?
        </div>

        <div className="modal-label">
          Recycle:
        </div>
        <div className="modal-block">
          <Checkbox
            value={recycle}
            onChange={setRecycle}
            label="Recycle resource, if not set - will be deleted permanently"
          />
        </div>

        {deleteError && (
          <div className="modal-error">
            {deleteError}
          </div>
        )}

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
