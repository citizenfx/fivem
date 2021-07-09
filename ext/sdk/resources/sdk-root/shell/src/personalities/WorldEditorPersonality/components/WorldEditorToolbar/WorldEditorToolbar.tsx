import React from 'react';
import { observer } from "mobx-react-lite";
import { BsX } from 'react-icons/bs';
// import { serverApi } from 'shared/api.events';
// import { sendApiMessage } from 'utils/api';
import { WEState } from '../../store/WEState';
import { StatusTool } from './StatusTool/StatusTool';
import { ModeSelector } from './ModeSelector/ModeSelector';
import { GameState } from 'store/GameState';
import { PatchesTool } from './PatchesTool/PatchesTool';
import { AdditionsTool } from './AdditionsTool/AdditionsTool';
import { AddObjectTool } from './AddObjectTool/AddObjectTool';
import s from './WorldEditorToolbar.module.scss';
import sBaseTool from './BaseTool/BaseTool.module.scss';

export const WorldEditorToolbar = observer(function WorldEditorToolbar() {
  const showControls = WEState.ready && !GameState.archetypesCollectionPending;

  return (
    <>
      <div className={s['top-left']}>
        {!!WEState.map && showControls && (
          <>
            <PatchesTool />
            <AdditionsTool />

            <div /> {/* gap */}

            <AddObjectTool />
          </>
        )}

        {/* <button onClick={() => sendApiMessage(serverApi.restartResource, 'sdk-game')}>
          restart sdk-game
        </button> */}
      </div>

      <div className={s['top-right']}>
        <StatusTool />

        <button
          className={sBaseTool.toggle}
          onClick={WEState.closeMap}
          data-label="Close World Editor"
        >
          <BsX />
        </button>
      </div>

      <div className={s.bottom}>
        {showControls && (
          <ModeSelector />
        )}
      </div>
    </>
  );
});
