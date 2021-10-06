import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { fileNamePattern } from 'constants/patterns';
import { FilesystemEntry } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { APIRQ } from 'shared/api.requests';
import { Api } from 'fxdk/browser/Api';
import s from './FileRenamer.module.scss';


export interface FileRenamerProps {
  entry: FilesystemEntry,
  onClose: () => void,
}

export const FileRenamer = React.memo(function FileRenamer(props: FileRenamerProps) {
  const { entry, onClose } = props;

  const [newFileName, setNewFileName] = React.useState(entry.name);

  const handleRename = React.useCallback(() => {
    onClose();

    Api.send(projectApi.renameEntry, {
      entryPath: entry.path,
      newName: newFileName,
    } as APIRQ.RenameEntry);
  }, [entry, newFileName, onClose]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Rename "{entry.name}" file
        </div>

        <Input
          autofocus
          value={newFileName}
          placeholder={entry.name}
          onChange={setNewFileName}
          pattern={fileNamePattern}
          className={s['name-input']}
          onSubmit={handleRename}
        />

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Rename file"
            onClick={handleRename}
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
