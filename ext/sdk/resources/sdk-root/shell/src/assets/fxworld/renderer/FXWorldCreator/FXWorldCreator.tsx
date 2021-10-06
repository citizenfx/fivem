import React from 'react';
import { observer } from 'mobx-react-lite';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { resourceNamePattern } from 'constants/patterns';
import { ProjectState } from 'store/ProjectState';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { APIRQ } from 'shared/api.requests';
import { assetTypes } from 'shared/asset.types';
import { assetApi } from 'shared/api.events';
import { Api } from 'fxdk/browser/Api';
import s from './FXWorldCreator.module.scss';

export interface FXWorldCreatorProps {
  close(): void;
}

export const FXWorldCreator = observer(function FXWorldCreator({ close }: FXWorldCreatorProps) {
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

    Api.send(assetApi.create, request);

    close();
  }, [mapName, close]);

  return (
    <Modal onClose={close}>
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
            onClick={close}
          />
        </div>
      </div>
    </Modal>
  );
});
