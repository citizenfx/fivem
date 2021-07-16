import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { ToolbarState } from 'store/ToolbarState';
import { GameView } from './components/GameView/GameView';
import { WEState } from './store/WEState';
import { WorldEditorToolbar } from './components/WorldEditorToolbar/WorldEditorToolbar';
import s from './WorldEditorPersonality.module.scss';
import { LoadScreen } from './components/LoadScreen/LoadScreen';
import { GameState } from 'store/GameState';
import { HTML5Backend } from 'react-dnd-html5-backend';
import { DndProvider } from 'react-dnd';

export const WorldEditorPersonality = observer(function WorldEditorPersonality() {
  const gameViewRef = React.useRef<HTMLDivElement>();

  const rootStyles: React.CSSProperties = {
    '--we-toolbar-width': `${WEState.mapExplorerWidth}px`,
  } as any;

  const rooClassName = classnames(s.root, {
    [s.fullwidth]: !ToolbarState.isOpen,
  });

  React.useEffect(() => {
    WEState.createInputController(gameViewRef);

    return () => WEState.destroyInputController();
  }, []);

  const showLoadScreen = !WEState.ready || GameState.archetypesCollectionPending;

  return (
    <div
      style={rootStyles}
      className={rooClassName}
    >
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
    </div>
  );
});
