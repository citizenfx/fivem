import React from 'react';
import classnames from 'classnames';
import { observer } from 'mobx-react-lite';
import { ToolbarState } from 'store/ToolbarState';
import { GameView } from './GameView/GameView';
import { WorldEditorState } from './WorldEditorState';
import { WorldEditorToolbar } from './WorldEditorToolbar/WorldEditorToolbar';
import s from './WorldEditorPersonality.module.scss';

export const WorldEditorPersonality = observer(function WorldEditorPersonality() {
  const gameViewRef = React.useRef<HTMLDivElement>();

  const rootStyles: React.CSSProperties = {
    '--toolbar-width': `${ToolbarState.width}px`,
  } as any;

  const rooClassName = classnames(s.root, {
    [s.fullwidth]: !ToolbarState.isOpen,
  });

  React.useEffect(() => {
    WorldEditorState.createInputController(gameViewRef);

    return () => WorldEditorState.destroyInputController();
  }, []);

  return (
    <div
      style={rootStyles}
      className={rooClassName}
    >
      <WorldEditorToolbar />

      <div
        ref={gameViewRef}
        className={s['game-view']}
      >
        <GameView />
      </div>
    </div>
  );
});
