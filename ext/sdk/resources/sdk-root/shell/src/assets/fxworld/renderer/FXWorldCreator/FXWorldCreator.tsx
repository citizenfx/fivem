import React from 'react';
import { observer } from 'mobx-react-lite';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { ProjectState } from 'store/ProjectState';
import s from './FXWorldCreator.module.scss';
import { Button } from 'components/controls/Button/Button';
import { APIRQ } from 'shared/api.requests';
import { assetTypes } from 'shared/asset.types';
import { sendApiMessage } from 'utils/api';
import { assetApi } from 'shared/api.events';

export const FXWorldCreator = observer(function FXWorldCreator() {
  const [mapName, setMapName] = React.useState('');

  const handleCreateMap = React.useCallback(() => {
    if (!mapName) {
      return;
    }

    const request: APIRQ.AssetCreate = {
      assetName: mapName,
      assetPath: ProjectState.project.path,
      assetType: assetTypes.fxworld,
    };

    sendApiMessage(assetApi.create, request);

    ProjectState.mapCreatorUI.close();
  }, [mapName]);

  return (
    <Modal onClose={ProjectState.mapCreatorUI.close}>
      <div className={s.root}>
        <div className="modal-header">
          Map creator
        </div>

        <div className="modal-block">
          <Input
            autofocus
            label="Map name"
            value={mapName}
            onChange={setMapName}
            pattern={resourceNamePattern}
            placeholder="Map name"
            onSubmit={handleCreateMap}
          />
        </div>

        <div className="modal-actions">
          <Button
            theme="primary"
            text="Create map"
            disabled={!mapName}
            onClick={handleCreateMap}
          />

          <Button
            text="Cancel"
            onClick={ProjectState.mapCreatorUI.close}
          />
        </div>
      </div>
    </Modal>
  );
});
