import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { APIRQ } from 'shared/api.requests';
import { projectApi } from 'shared/api.events';
import { FXWORLD_FILE_EXT } from 'assets/fxworld/fxworld-types';
import { Api } from 'fxdk/browser/Api';
import s from './FXWorldRenamer.module.scss';


export interface FXWorldRenamerProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const FXWorldRenamer = React.memo(function FXWorldRenamer({ name, path, onClose }: FXWorldRenamerProps) {
  const [newName, setNewName] = React.useState(name);

  const handleRenameMap = React.useCallback(() => {
    const request: APIRQ.RenameEntry = {
      entryPath: path,
      newName: newName + FXWORLD_FILE_EXT,
    };

    Api.send(projectApi.renameEntry, request);

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
