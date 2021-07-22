import React from 'react';
import { IoBandageSharp } from 'react-icons/io5';
import { WETool } from '../../../store/WEToolbarState';
import { BaseTool } from '../BaseTool/BaseTool';
import { observer } from 'mobx-react-lite';
import { Patch } from './Patch';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import s from './PatchesTool.module.scss';
import { WESelectionType } from 'backend/world-editor/world-editor-types';
import { patchesToolIcon } from 'personalities/WorldEditorPersonality/constants/icons';

export const PatchesTool = observer(function PatchesTool() {
  return (
    <BaseTool
      tool={WETool.Patches}
      icon={patchesToolIcon}
      label="Map patches"
      highlight={WEState.selection.type === WESelectionType.PATCH}
    >
      <div className={s.root}>
        {Object.keys(WEState.map.patches).length === 0 && (
          <div className={s.placeholder}>
            Modify map objects and their patches will appear here!
          </div>
        )}

        {Object.entries(WEState.map.patches).map(([mapData, entities]) => Object.keys(entities).map((id) => {
          return (
            <Patch
              key={mapData + id}
              mapData={mapData}
              entityId={id}
            />
          );
        }))}
      </div>
    </BaseTool>
  );
});
