import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { APIRQ } from 'shared/api.requests';
import { projectApi } from 'shared/api.events';
import { Api } from 'fxdk/browser/Api';
import s from './ResourceRenamer.module.scss';


export interface ResourceRenamerProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const ResourceRenamer = React.memo(function ResourceRenamer({ name, path, onClose }: ResourceRenamerProps) {
  const [newName, setNewName] = React.useState(name);

  const handleRenameResource = React.useCallback(() => {
    const request: APIRQ.RenameEntry = {
      entryPath: path,
      newName,
    };

    Api.send(projectApi.renameEntry, request);

    onClose();
  }, [path, newName, onClose]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Rename "{name}" Resource
        </div>

        <Input
          autofocus
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
