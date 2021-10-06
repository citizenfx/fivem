import React from 'react';
import { observer } from 'mobx-react-lite';
import { GameView } from './components/GameView/GameView';
import { WEState } from './store/WEState';
import { WorldEditorToolbar } from './components/WorldEditorToolbar/WorldEditorToolbar';
import { LoadScreen } from './components/LoadScreen/LoadScreen';
import { GameState } from 'store/GameState';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { DndProvider } from 'react-dnd';
import { WorldEditorIntro } from './components/WorldEditorIntro/WorldEditorIntro';
import s from './WorldEditorPersonality.module.scss';

export const WorldEditorPersonality = observer(function WorldEditorPersonality() {
  const gameViewRef = React.useRef<HTMLDivElement>(null);

  React.useEffect(() => {
    WEState.createInputController(gameViewRef);

    return () => WEState.destroyInputController();
  }, []);

  const showLoadScreen = !WEState.ready || GameState.archetypesCollectionPending;

  const showIntro = !GameState.archetypesCollectionPending && WEState.showIntro;

  return (
    <div className={s.root}>
      <DndProvider backend={HTML5Backend}>
        <WorldEditorToolbar />
      </DndProvider>

      <div
        ref={gameViewRef}
        className={s['game-view']}
      >
        <GameView />
      </div>

      {showLoadScreen && (
        <LoadScreen />
      )}

      {showIntro && (
        <WorldEditorIntro />
      )}
    </div>
  );
});
