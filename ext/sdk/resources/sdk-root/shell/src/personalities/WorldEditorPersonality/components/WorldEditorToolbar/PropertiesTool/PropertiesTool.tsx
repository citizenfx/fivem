import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { WESelectionType } from 'backend/world-editor/world-editor-types';
import { PatchProperties } from './PatchProperties';
import { AdditionProprties } from './AdditionProperties';
import { WEToolbarState } from 'personalities/WorldEditorPersonality/store/WEToolbarState';
import { FakeProperties } from './FakeProperties';
import s from './PropertiesTool.module.scss';
import sBaseTool from '../BaseTool/BaseTool.module.scss'

export const PropertiesTool = observer(function PropertiesTool() {
  const rootClassName = classnames(s.root, sBaseTool['panel-alike']);

  if (WEToolbarState.showingFakeProperties) {
    return (
      <div className={rootClassName} data-intro-id="properties-panel">
        <FakeProperties />
      </div>
    );
  }

  if (WEState.selection.type === WESelectionType.ADDITION) {
    if (!WEState.map.additions[WEState.selection.id]) {
      return null;
    }

    return (
      <div className={rootClassName}>
        <AdditionProprties
          additionId={WEState.selection.id}
        />
      </div>
    );
  }

  if (WEState.selection.type === WESelectionType.PATCH) {
    return (
      <div className={rootClassName}>
        <PatchProperties />
      </div>
    );
  }

  return null;
});
