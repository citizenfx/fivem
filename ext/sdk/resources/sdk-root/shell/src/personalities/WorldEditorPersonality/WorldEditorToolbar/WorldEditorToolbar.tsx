import React from 'react';
import { observer } from "mobx-react-lite";
import { BsX } from 'react-icons/bs';
// import { serverApi } from 'shared/api.events';
// import { sendApiMessage } from 'utils/api';
import { WorldEditorState } from '../WorldEditorState';
import { StatusBar } from './StatusBar/StatusBar';
import { ObjectsBrowser } from './ObjectsBrowser/ObjectsBrowser';
import { ModeSelector } from './ModeSelector/ModeSelector';
import { MapExplorer } from './MapExplorer/MapExplorer';
import s from './WorldEditorToolbar.module.scss';

export const WorldEditorToolbar = observer(function WorldEditorToolbar() {
  return (
    <div className={s.root}>
      <div className={s.left}>
        {WorldEditorState.ready && (
          <MapExplorer />
        )}

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
