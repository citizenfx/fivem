import { observer } from 'mobx-react-lite';
import { WEMode, WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import React from 'react';
import s from './PlayExitMessage.module.scss';

export const PlayExitMessage = observer(function PlayExitMessage() {
  if (WEState.mode !== WEMode.PLAYTEST) {
    return null;
  }

  return (
    <div className={s.root}>
      <div className={s.message}>
        Press ESC to exit play mode
      </div>
    </div>
  );
});
