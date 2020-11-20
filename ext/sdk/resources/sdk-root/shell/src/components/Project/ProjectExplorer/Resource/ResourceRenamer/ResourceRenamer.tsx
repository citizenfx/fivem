import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { AssetRenameRequest } from 'shared/api.types';
import { assetApi } from 'shared/events';
import { sendApiMessage } from 'utils/api';
import s from './ResourceRenamer.module.scss';


export interface ResourceRenamerProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const ResourceRenamer = React.memo(function ResourceRenamer({ name, path, onClose }: ResourceRenamerProps) {
  const [newName, setNewName] = React.useState(name);

  const handleRenameResource = React.useCallback(() => {
    const request: AssetRenameRequest = {
      assetPath: path,
      newAssetName: newName,
    };

    sendApiMessage(assetApi.rename, request);

    onClose();
  }, [path, newName, onClose]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Rename "{name}" Resource
        </div>

        <Input
          pattern={resourceNamePattern}
          onChange={setNewName}
          onSubmit={handleRenameResource}
          value={newName}
          className={s['name-input']}
        />

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Rename resource"
            onClick={handleRenameResource}
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
