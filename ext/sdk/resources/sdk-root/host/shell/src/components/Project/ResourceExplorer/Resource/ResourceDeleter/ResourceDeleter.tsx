import React from 'react';
import { ProjectContext } from '../../../../../contexts/ProjectContext';
import { AssetDeleteRequest } from '../../../../../sdkApi/api.types';
import { assetApi } from '../../../../../sdkApi/events';
import { sendApiMessage } from '../../../../../utils/api';
import { invariant } from '../../../../../utils/invariant';
import { Button } from '../../../../controls/Button/Button';
import { Modal } from '../../../../Modal/Modal';

import s from './ResourceDeleter.module.scss';


export interface ResourceDeleterProps {
  onClose: () => void,
  name: string,
  path: string,
}

export const ResourceDeleter = React.memo(({ name, path, onClose }: ResourceDeleterProps) => {
  const { project } = React.useContext(ProjectContext);
  invariant(project, 'No project');

  const handleDeleteResource = React.useCallback(() => {
    const request: AssetDeleteRequest = {
      projectPath: project.path,
      assetPath: path,
    };

    sendApiMessage(assetApi.delete, request);
    onClose();
  }, [path, project, onClose]);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Delete "{name}" Resource?
        </div>

        <div className={s.disclaimer}>
          This action can not be reverted. Are you sure?
        </div>

        <div className="modal-actions">
          <Button
            text="Delete resource"
            onClick={handleDeleteResource}
          />

          <Button
            theme="primary"
            text="Cancel"
            onClick={onClose}
          />
        </div>
      </div>
    </Modal>
  );
});
