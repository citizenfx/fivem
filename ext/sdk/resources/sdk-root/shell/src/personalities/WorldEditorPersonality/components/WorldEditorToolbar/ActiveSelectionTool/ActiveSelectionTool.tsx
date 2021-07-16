import React from 'react';
import classnames from 'classnames';
import sBaseTool from '../BaseTool/BaseTool.module.scss';
import s from './ActiveSelectionTool.module.scss';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { WESelectionType } from 'backend/world-editor/world-editor-types';
import { additionsToolIcon, patchesToolIcon } from 'personalities/WorldEditorPersonality/constants/icons';
import { closeIcon } from 'constants/icons';

function getIconNode(): React.ReactNode {
  switch (WEState.selection.type) {
    case WESelectionType.ADDITION: {
      return additionsToolIcon;
    }

    case WESelectionType.PATCH: {
      return patchesToolIcon;
    }

    default: {
      return null;
    }
  }
}

export const ActiveSelectionTool = observer(function ActiveSelectionTool() {
  if (WEState.selection.type === WESelectionType.NONE) {
    return null;
  }

  const rootClassName = classnames(sBaseTool.toggle, s.root);

  let label: string;
  if (WEState.selection.type === WESelectionType.ADDITION) {
    label = WEState.map.additions[WEState.selection.id].label;
  } else {
    const patch = WEState.map.getPatch(WEState.selection.mapdata, WEState.selection.entity);

    label = patch
      ? patch.label
      : WEState.selection.label;
  }

  return (
    <div className={rootClassName}>
      <div className={s.icon}>
        {getIconNode()}
      </div>
      <div className={s.label}>
        {label}
      </div>
      <button className={s.clear} onClick={WEState.clearEditorSelection} data-label="Clear selection (ESC)">
        {closeIcon}
      </button>
    </div>
  );
});
