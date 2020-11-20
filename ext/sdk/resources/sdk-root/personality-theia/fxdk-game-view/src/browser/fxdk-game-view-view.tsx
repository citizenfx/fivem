import * as React from 'react';
import { ReactWidget } from '@theia/core/lib/browser';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import { injectable, postConstruct } from 'inversify';

import './common/game-view.webcomponent.js';


const stylesheet = /*css*/`
  .fxdk-game-view {
    display: flex;
    flex-direction: column;
    width: 100%;
    height: 100%;
    outline: none;
  }
  .fxdk-game-view:hover .fxdk-play-button {
    transform: translateY(0);
  }

  .fxdk-game-bar-wrapper {
    position: absolute;
    top: 0;
    left: 0;
    right: 0;

    display: flex;
    justify-content: center;

    z-index: 10;
  }

  .fxdk-game-bar {
  }

  .fxdk-play-button {
    padding: 5px 15px;

    backdrop-filter: blur(20px);
    background-color: rgba(255, 255, 255, .25);

    color: white;
    font-size: 14px;
    text-transform: uppercase;
    font-family: 'Segoe UI';
    font-weight: 500;
    letter-spacing: 1px;
    text-shadow: 0 0 2px rgba(0,0,0,.25);

    border: solid 1px rgba(255, 255, 255, .1);
    border-top: none;
    border-bottom-left-radius: 4px;
    border-bottom-right-radius: 4px;

    cursor: pointer;
    outline: none;

    transform: translateY(-100%);
    transition: transform .2s ease;
  }
  .fxdk-play-button:hover {
    background-color: rgba(255, 255, 255, .45);
  }

  .fxdk-game-view game-view {
    flex-grow: 1;
    width: 100%;
  }
`;

const GameView = React.memo(() => {
  const ref = React.useRef<any>(null);

  const enterFullscreen = React.useCallback(() => {
    if (ref.current) {
      ref.current.enterFullscreenControlingMode();
    }
  }, []);

  return (
    <div tabIndex={0} className="fxdk-game-view">
      <div className="fxdk-game-bar-wrapper">
        <div className="fxdk-game-bar">
          <button onClick={enterFullscreen} className="fxdk-play-button">
            Go fullscreen
          </button>
        </div>
      </div>
      <game-view ref={ref}></game-view>
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
      <>
        <style>
          {stylesheet}
        </style>
        <GameView />
      </>
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
