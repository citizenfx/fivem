import React from 'react';
import { observer } from "mobx-react-lite";
import { WorldEditorState } from '../WorldEditorState';
import { BsX } from 'react-icons/bs';
import { BiWorld } from 'react-icons/bi';
import { StatusBar } from './StatusBar/StatusBar';
import { ObjectsBrowser } from './ObjectsBrowser/ObjectsBrowser';
import { ModeSelector } from './ModeSelector/ModeSelector';
import s from './WorldEditorToolbar.module.scss';

// import { serverApi } from 'shared/api.events';
// import { sendApiMessage } from 'utils/api';

export const WorldEditorToolbar = observer(function WorldEditorToolbar() {
  return (
    <div className={s.root}>
      <div className={s.left}>
        <div className={s.header}>
          <div className={s.name}>
            <BiWorld />
            {WorldEditorState.mapName}
          </div>
        </div>

        {WorldEditorState.ready && (
          <ObjectsBrowser />
        )}

        {/* <button onClick={() => sendApiMessage(serverApi.restartResource, 'sdk-game')}>
          restart sdk-game
        </button> */}
      </div>

      <div className={s.center}>
        {WorldEditorState.ready && (
          <ModeSelector />
        )}
      </div>

      <div className={s.right}>
        <StatusBar />

        <button
          className={s.close}
          onClick={WorldEditorState.closeMap}
        >
          <BsX />
        </button>
      </div>
    </div>
  );
});
