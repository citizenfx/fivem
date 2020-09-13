import React from 'react';
import { resourceNamePattern } from '../../../../../constants/patterns';
import { ProjectContext } from '../../../../../contexts/ProjectContext';
import { AssetRenameRequest } from '../../../../../sdkApi/api.types';
import { assetApi } from '../../../../../sdkApi/events';
import { sendApiMessage } from '../../../../../utils/api';
import { invariant } from '../../../../../utils/invariant';
import { Button } from '../../../../controls/Button/Button';
import { Input } from '../../../../controls/Input/Input';
import { Modal } from '../../../../Modal/Modal';

import s from './ResourceRenamer.module.scss';


export interface ResourceRenamerProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const ResourceRenamer = React.memo(({ name, path, onClose }: ResourceRenamerProps) => {
  const { project } = React.useContext(ProjectContext);
  invariant(project, 'No project');

  const [newName, setNewName] = React.useState(name);

  const handleRenameResource = React.useCallback(() => {
    const request: AssetRenameRequest = {
      projectPath: project.path,
      assetPath: path,
      newAssetName: newName,
    };

    sendApiMessage(assetApi.rename, request);

    onClose();
  }, [path, project, newName, onClose]);

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
