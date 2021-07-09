import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import s from './PatchesTool.module.scss';

export interface PatchProps {
  mapData: string,
  entityId: string,
}

export const Patch = observer(function Patch(props: PatchProps) {
  const { mapData, entityId } = props;
  const patch = WEState.map.patches[mapData][entityId];

  return (
    <div className={s.item} onClick={() => WEState.setCam(patch.cam)}>
      {patch.label}
    </div>
  );
});
