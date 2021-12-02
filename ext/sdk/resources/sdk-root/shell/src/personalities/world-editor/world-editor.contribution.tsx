import React from 'react';
import { IShellViewParticipant, ShellViewParticipants } from "fxdk/browser/shellExtensions";
import { ShellState } from "store/ShellState";
import s from './WorldEditorPersonality.module.scss';
import { LoadScreen } from './components/LoadScreen/LoadScreen';

const WELoadScreen = (
  <div className={s.root}>
    <LoadScreen />
  </div>
);

const WorldEditorPersonality = React.lazy(async () => {
  const { WorldEditorPersonality } = await import(
    /* webpackPrefetch: true, webpackChunkName: "world-editor-personality" */
    './WorldEditorPersonality'
  );

  return {
    default: WorldEditorPersonality,
  };
});

ShellViewParticipants.register(new class WorldEditorView implements IShellViewParticipant {
  readonly id = 'world-editor';

  isVisible() {
    return ShellState.isWorldEditorPersonality;
  }

  render() {
    return (
      <React.Suspense fallback={WELoadScreen}>
        <WorldEditorPersonality />
      </React.Suspense>
    );
  }
});
