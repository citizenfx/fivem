import * as React from 'react';
import { ReactWidget } from '@theia/core/lib/browser';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import { injectable, postConstruct } from 'inversify';

import './common/game-view.webcomponent.js';

const GameView = React.memo(() => {
  const [pointerLocked, setPointerLocked] = React.useState(false);
  const gameViewRef = React.useRef<any>(null);

  const enterFullscreen = React.useCallback(() => {
    if (gameViewRef.current) {
      gameViewRef.current.enterFullscreen();
      gameViewRef.current.lockPointer();
    }
  }, []);

  React.useEffect(() => {
    const pointerlockchangeCallback = (e) => {
      setPointerLocked(e.detail.locked);
    };

    if (gameViewRef.current) {
      gameViewRef.current.addEventListener('pointerlockchange', pointerlockchangeCallback);
    }

    return () => {
      if (gameViewRef.current) {
        gameViewRef.current.removeEventListener('pointerlockchange', pointerlockchangeCallback);
      }
    };
  }, []);

  return (
    <div tabIndex={0} className="fxdk-game-view">
      <div className="fxdk-game-bar-wrapper">
        <div className={`fxdk-game-bar ${pointerLocked ? 'fxdk-pointer-locked' : ''}`}>
          <button
            onClick={enterFullscreen}
            className="fxdk-play-button"
          >
            Go fullscreen
          </button>
        </div>
      </div>
      <game-view ref={gameViewRef}></game-view>
    </div>
  );
});

@injectable()
export class FxdkGameView extends ReactWidget {
  static readonly ID = 'fxdkGameView';

  @postConstruct()
  init(): void {
    this.id = FxdkGameView.ID;
    this.title.closable = true;
    this.title.caption = 'Game view';
    this.title.label = 'Game view';
    this.title.iconClass = 'fa fa-gamepad';
    this.update();
  }

  protected onActivateRequest() {
    this.node.focus();
  }

  protected render(): React.ReactNode {
    return (
      <GameView />
    );
  }
}

@injectable()
export class FxdkGameViewContribution extends AbstractViewContribution<FxdkGameView> {
  static readonly FXDK_GAME_VIEW_TOGGLE_COMMAND_ID = 'fxdkGameView:toggle';

  constructor() {
    super({
      widgetId: FxdkGameView.ID,
      widgetName: 'Game View',
      toggleCommandId: FxdkGameViewContribution.FXDK_GAME_VIEW_TOGGLE_COMMAND_ID,
      defaultWidgetOptions: {
        area: 'main',
      }
    });
  }
}
