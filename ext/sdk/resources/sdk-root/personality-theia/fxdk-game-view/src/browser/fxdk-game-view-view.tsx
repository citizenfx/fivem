import * as React from 'react';
import { ReactWidget } from '@theia/core/lib/browser';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import { inject, injectable, postConstruct } from 'inversify';
import { FxdkDataService, GameStates } from 'fxdk-services/lib/browser/fxdk-data-service';

import './common/game-view.webcomponent.js';

const gameStateToCaption: Record<GameStates, string> = {
  [GameStates.NOT_RUNNING]: 'not running',
  [GameStates.READY]: 'ready',
  [GameStates.LOADING]: 'loading',
  [GameStates.UNLOADING]: 'unloading',
  [GameStates.CONNECTED]: 'active'
};

const GameView = React.memo(({ gameState }: { gameState: GameStates }) => {
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

  @inject(FxdkDataService)
  protected readonly dataService: FxdkDataService;

  private theiaIsActive = false;
  private gameState = GameStates.NOT_RUNNING;

  @postConstruct()
  init(): void {
    this.id = FxdkGameView.ID;
    this.title.closable = true;
    this.title.caption = 'Game view';
    this.title.iconClass = 'fa fa-gamepad';

    this.theiaIsActive = this.dataService.getTheiaIsActive();
    this.toDispose.push(this.dataService.onTheiaIsActiveChange((theiaIsActive) => {
      this.theiaIsActive = theiaIsActive;
      this.update();
    }));

    this.gameState = this.dataService.getGameState();
    this.title.label = 'Game - ' + gameStateToCaption[this.gameState];

    this.toDispose.push(this.dataService.onGameStateChange((gameState) => {
      this.title.label = 'Game - ' + gameStateToCaption[gameState];
      this.title.caption = gameStateToCaption[gameState];

      if (gameState === GameStates.LOADING || gameState === GameStates.UNLOADING) {
        this.title.iconClass = 'fa fa-spinner fa-spin';
      } else {
        this.title.iconClass = 'fa fa-gamepad';
      }

      this.gameState = gameState;
      this.update();
    }));

    this.update();
  }

  protected onActivateRequest() {
    this.node.focus();
  }

  protected render(): React.ReactNode {
    if (!this.theiaIsActive) {
      return (
        <div>
          Theia is in inactive state now, and you definitely shouldn't have seen this.
        </div>
      );
    }

    return (
      <GameView gameState={this.gameState} />
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
