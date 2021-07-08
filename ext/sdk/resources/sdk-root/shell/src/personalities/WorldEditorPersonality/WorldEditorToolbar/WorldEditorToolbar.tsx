import React from 'react';
import { observer } from "mobx-react-lite";
import { BsX } from 'react-icons/bs';
// import { serverApi } from 'shared/api.events';
// import { sendApiMessage } from 'utils/api';
import { WorldEditorState } from '../WorldEditorState';
import { StatusBar } from './StatusBar/StatusBar';
import { ModeSelector } from './ModeSelector/ModeSelector';
import { MapExplorer } from './MapExplorer/MapExplorer';
import s from './WorldEditorToolbar.module.scss';
import { GameState } from 'store/GameState';

export const WorldEditorToolbar = observer(function WorldEditorToolbar() {
  const showControls = WorldEditorState.ready && !GameState.archetypesCollectionPending;

  return (
    <div className={s.root}>
      <div className={s.left}>
        {showControls && (
          <MapExplorer />
        )}

        {/* <button onClick={() => sendApiMessage(serverApi.restartResource, 'sdk-game')}>
          restart sdk-game
        </button> */}
      </div>

      <div className={s.center}>
        {showControls && (
          <ModeSelector />
        )}
      </div>

      <div className={s.right}>
        <StatusBar />

        <button
          className={s.close}
          onClick={WorldEditorState.closeMap}
          data-label="Close World Editor"
        >
          <BsX />
        </button>
      </div>
    </div>
  );
});
