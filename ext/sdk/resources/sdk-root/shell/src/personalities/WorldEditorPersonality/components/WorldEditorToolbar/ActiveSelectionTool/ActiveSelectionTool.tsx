import React from 'react';
import classnames from 'classnames';
import sBaseTool from '../BaseTool/BaseTool.module.scss';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { WESelectionType } from 'backend/world-editor/world-editor-types';
import { additionsToolIcon, patchesToolIcon } from 'personalities/WorldEditorPersonality/constants/icons';
import { closeIcon } from 'constants/icons';
import { Title } from 'components/controls/Title/Title';
import s from './ActiveSelectionTool.module.scss';

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

function getSelectionLabel(): string {
  switch (WEState.selection.type) {
    case WESelectionType.NONE: {
      return '';
    }

    case WESelectionType.ADDITION: {
      return WEState.map!.additions[WEState.selection.id]?.label || '';
    }

    case WESelectionType.PATCH: {
      const patch = WEState.map!.getPatch(WEState.selection.mapdata, WEState.selection.entity);

      return patch
        ? patch.label
        : WEState.selection.label;
    }
  }
}

export const ActiveSelectionTool = observer(function ActiveSelectionTool() {
  if (WEState.selection.type === WESelectionType.NONE) {
    return null;
  }

  const rootClassName = classnames(sBaseTool.toggle, s.root);

  return (
    <div className={rootClassName}>
      <div className={s.icon}>
        {getIconNode()}
      </div>
      <div className={s.label}>
        {getSelectionLabel()}
      </div>
      <Title animated={false} delay={0} fixedOn="top" title="Clear selection" shortcut="ESC">
        {(ref) => (
          <button
            ref={ref}
            className={s.clear}
            onClick={WEState.clearEditorSelection}
          >
            {closeIcon}
          </button>
        )}
      </Title>
    </div>
  );
});
