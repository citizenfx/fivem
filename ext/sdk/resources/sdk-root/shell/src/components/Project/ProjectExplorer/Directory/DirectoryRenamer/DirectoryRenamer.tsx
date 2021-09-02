import * as React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { fileNamePattern } from 'constants/patterns';
import { FilesystemEntry } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { APIRQ } from 'shared/api.requests';
import s from './DirectoryRenamer.module.scss';


export interface DirectoryRenamerProps {
  entry: FilesystemEntry,
  onClose: () => void,
}

export const DirectoryRenamer = React.memo(function DirectoryRenamer(props: DirectoryRenamerProps) {
  const { entry, onClose } = props;

  const [newDirectoryName, setNewDirectoryName] = React.useState(entry.name);

  const handleRename = React.useCallback(() => {
    onClose();

    sendApiMessage(projectApi.renameEntry, {
      entryPath: entry.path,
      newName: newDirectoryName,
    } as APIRQ.RenameEntry);
  }, [entry, newDirectoryName, onClose]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Rename "{entry.name}" directory
        </div>

        <Input
          autofocus
          value={newDirectoryName}
          placeholder={entry.name}
          onChange={setNewDirectoryName}
          pattern={fileNamePattern}
          className={s['name-input']}
          onSubmit={handleRename}
        />

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Rename directory"
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
