import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { AssetRenameRequest } from 'shared/api.requests';
import { assetApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import s from './FXWorldRenamer.module.scss';
import { FXWORLD_FILE_EXT } from 'assets/fxworld/fxworld-types';


export interface FXWorldRenamerProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const FXWorldRenamer = React.memo(function FXWorldRenamer({ name, path, onClose }: FXWorldRenamerProps) {
  const [newName, setNewName] = React.useState(name);

  const handleRenameMap = React.useCallback(() => {
    const request: AssetRenameRequest = {
      assetPath: path,
      newAssetName: newName + FXWORLD_FILE_EXT,
    };

    sendApiMessage(assetApi.rename, request);

    onClose();
  }, [path, newName, onClose]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Rename "{name}" Map
        </div>

        <Input
          autofocus
          pattern={resourceNamePattern}
          onChange={setNewName}
          onSubmit={handleRenameMap}
          value={newName}
          className={s['name-input']}
        />

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Rename map"
            onClick={handleRenameMap}
            disabled={!newName}
          />

          <Button
            text="Cancel"
            onClick={onClose}
          />
        </div>
      </div>
    </Modal>
  );
});
