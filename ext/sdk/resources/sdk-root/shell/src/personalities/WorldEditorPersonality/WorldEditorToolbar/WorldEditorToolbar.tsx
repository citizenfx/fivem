import React from 'react';
import classnames from 'classnames';
import { observer } from "mobx-react-lite";
import { EditorMode, WorldEditorState } from '../WorldEditorState';
import { BsBoundingBoxCircles, BsX } from 'react-icons/bs';
import { CgArrowsExpandUpRight } from 'react-icons/cg';
import { GiResize } from 'react-icons/gi';
import { AiOutlineRotateRight } from 'react-icons/ai';
import { BiWorld } from 'react-icons/bi';
import { StatusBar } from './StatusBar/StatusBar';
import { ObjectsBrowser } from './ObjectsBrowser/ObjectsBrowser';
import s from './WorldEditorToolbar.module.scss';

// import { serverApi } from 'shared/api.events';
// import { sendApiMessage } from 'utils/api';

export const WorldEditorToolbar = observer(function WorldEditorToolbar() {
  return (
    <div className={s.root}>
      <div>
        <div className={s.header}>
          <button
            className={s.close}
            onClick={WorldEditorState.closeMap}
          >
            <BsX />
          </button>
          <div className={s.name}>
            <BiWorld />
            {WorldEditorState.mapFile}
          </div>
        </div>

        {/* <button onClick={() => sendApiMessage(serverApi.restartResource, 'sdk-game')}>
          restart sdk-game
        </button> */}
      </div>

      <div>
        <div className={s.modes}>
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
      </div>

      <div>
        <ObjectsBrowser />

        <StatusBar />
      </div>
    </div>
  );
});
