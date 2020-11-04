import * as React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { fileNamePattern } from 'constants/patterns';
import { FilesystemEntry } from 'sdkApi/api.types';
import { projectApi } from 'sdkApi/events';
import { sendApiMessage } from 'utils/api';
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

    sendApiMessage(projectApi.renameFile, {
      filePath: entry.path,
      newFileName,
    });
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
