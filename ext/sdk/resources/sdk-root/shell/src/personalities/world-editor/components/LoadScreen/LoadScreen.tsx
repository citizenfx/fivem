import React from 'react';
import { observer } from 'mobx-react-lite';
import classnames from 'classnames';
import { GameState } from 'store/GameState';
import { GameLoadingState } from 'store/GameLoadingState';
import { GameStates } from 'backend/game/game-constants';
import s from './LoadScreen.module.scss';

function getLoadingText() {
  if (GameState.state === GameStates.READY) {
    return 'The World Editor is preparing to load...';
  }

  if (GameState.state === GameStates.UNLOADING) {
    return 'The World Editor is getting ready...';
  }

  if (GameState.archetypesCollectionPending) {
    return 'Building object list...';
  }

  return 'Loading World Editor...';
}

export const LoadScreen = observer(function LoadScreen() {
  const text = getLoadingText();

  const progress = GameState.state !== GameStates.LOADING
    ? '0%'
    : `${GameLoadingState.loadingProgress * 100}%`;
  const progressClassName = classnames(s.progress, {
    [s.indeterminate]: GameLoadingState.loadingProgress === 0 || GameState.state !== GameStates.LOADING,
  });

  return (
    <div className={classnames(s.root, 'animated-background')}>
      <div className={s.text}>
        {text}
      </div>

      <div className={progressClassName}>
        <div className={s.bar} style={{ width: progress }} />
      </div>
    </div>
  );
});
