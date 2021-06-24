import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { CgArrowsExpandUpRight } from 'react-icons/cg';
import { EditorMode, WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import { AiOutlineRotateRight } from 'react-icons/ai';
import { GiResize } from 'react-icons/gi';
import { BsBoundingBoxCircles } from 'react-icons/bs';
import s from './ModeSelector.module.scss';

export const ModeSelector = observer(function ModeSelector() {
  return (
    <div className={s.root}>
      <div
        onClick={WorldEditorState.enableTranslation}
        className={classnames(s.mode, { [s.active]: WorldEditorState.editorMode === EditorMode.TRANSLATE })}
      >
        <CgArrowsExpandUpRight />
        <span>
          Translate
          <div className="shortcut">1</div>
        </span>
      </div>
      <div
        onClick={WorldEditorState.enableRotation}
        className={classnames(s.mode, { [s.active]: WorldEditorState.editorMode === EditorMode.ROTATE })}
      >
        <AiOutlineRotateRight />
        <span>
          Rotate
          <div className="shortcut">2</div>
        </span>
      </div>
      <div
        onClick={WorldEditorState.enableScaling}
        className={classnames(s.mode, { [s.active]: WorldEditorState.editorMode === EditorMode.SCALE })}
      >
        <GiResize />
        <span>
          Scale
          <div className="shortcut">3</div>
        </span>
      </div>

      <div className={classnames(s.mode, { [s.active]: WorldEditorState.editorLocal })} onClick={WorldEditorState.toggleEditorLocal}>
        <BsBoundingBoxCircles />
        <span>
          Local coords
          <div className="shortcut">~</div>
        </span>
      </div>
    </div>
  );
});
