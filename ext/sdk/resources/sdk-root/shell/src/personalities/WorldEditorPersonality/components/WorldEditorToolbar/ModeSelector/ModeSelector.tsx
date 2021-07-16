import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { CgArrowsExpandUpRight } from 'react-icons/cg';
import { EditorMode, WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { AiOutlineRotateRight } from 'react-icons/ai';
import { GiResize } from 'react-icons/gi';
import s from './ModeSelector.module.scss';

export const ModeSelector = observer(function ModeSelector() {
  return (
    <div className={s.root}>
      <div className={s.toggle} onClick={WEState.toggleEditorLocal}>
        <div className={classnames(s.coords, { [s.global]: !WEState.editorLocal })}>
          <div className={s.axis} />
          <div className={s.object} />
        </div>

        <span>
          Switch to {WEState.editorLocal ? 'global' : 'local'} coords
          <div className="shortcut">~</div>
        </span>
      </div>

      <div className={s.modes}>
        <div
          onClick={WEState.enableTranslation}
          className={classnames(s.mode, { [s.active]: WEState.editorMode === EditorMode.TRANSLATE })}
        >
          <CgArrowsExpandUpRight />
          <span>
            Translate
            <div className="shortcut">1</div>
          </span>
        </div>
        <div
          onClick={WEState.enableRotation}
          className={classnames(s.mode, { [s.active]: WEState.editorMode === EditorMode.ROTATE })}
        >
          <AiOutlineRotateRight />
          <span>
            Rotate
            <div className="shortcut">2</div>
          </span>
        </div>
        <div
          onClick={WEState.enableScaling}
          className={classnames(s.mode, { [s.active]: WEState.editorMode === EditorMode.SCALE })}
        >
          <GiResize />
          <span>
            Scale
            <div className="shortcut">3</div>
          </span>
        </div>
      </div>
    </div>
  );
});
