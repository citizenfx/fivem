import React from 'react';
import { observer } from 'mobx-react-lite';
import { Button } from 'components/controls/Button/Button';
import { Modal } from 'components/Modal/Modal';
import { AssetDeleteRequest, AssetDeleteResponse } from 'shared/api.requests';
import { assetApi } from 'shared/api.events';
import { useSendApiMessageCallback } from 'utils/hooks';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import { ProjectState } from 'store/ProjectState';
import s from './FXWorldDeleter.module.scss';


export interface FXWorldDeleterProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const FXWorldDeleter = observer(function FXWorldDeleter({ name, path, onClose }: FXWorldDeleterProps) {
  const [recycle, setRecycle] = React.useState(true);
  const [deleteError, setDeleteError] = React.useState('');

  const deleteMap = useSendApiMessageCallback<AssetDeleteRequest, AssetDeleteResponse>(assetApi.delete, (error, response) => {
    if (error) {
      return setDeleteError(error);
    }

    if (response === AssetDeleteResponse.FailedToRecycle) {
      return setDeleteError('Failed to recycle, most likely due to disabled recycle bin, try again without recycle option set or enable recycle bin');
    }

    onClose();
  });

  const handleDeleteMap = React.useCallback(() => {
    setDeleteError('');

    ProjectState.addPendingFolderDeletion(path);

    const request: AssetDeleteRequest = {
      assetPath: path,
      hardDelete: !recycle,
    };

    deleteMap(request);

    onClose();
  }, [path, recycle, setDeleteError, deleteMap]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Delete "{name}" Map?
        </div>

        <div className="modal-label">
          Recycle:
        </div>
        <div className="modal-block">
          <Checkbox
            value={recycle}
            onChange={setRecycle}
            label="Recycle map, if not set - will be deleted permanently"
          />
        </div>

        {deleteError && (
          <div className="modal-error">
            {deleteError}
          </div>
        )}

        <div className="modal-actions">
          <Button
            text="Delete map"
            onClick={handleDeleteMap}
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
