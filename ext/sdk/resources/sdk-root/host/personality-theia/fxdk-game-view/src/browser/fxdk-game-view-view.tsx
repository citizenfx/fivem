import * as React from 'react';
import { ReactWidget } from '@theia/core/lib/browser';
import { AbstractViewContribution } from '@theia/core/lib/browser';
import { injectable, postConstruct } from 'inversify';

import './common/game-view.webcomponent.js';

const GameView = React.memo(() => {
  const ref = React.useRef<any>(null);

  const [pos, setPos] = React.useState([]);

  // Setup message listener
  React.useEffect(() => {
    const handleMessage = (e) => {
      if (e.data.type === 'pos') {
        setPos(e.data.payload);
      }
    };

    window.addEventListener('message', handleMessage);

    return () => window.removeEventListener('message', handleMessage);
  }, []);

  const enterFullscreen = React.useCallback(() => {
    if (ref.current) {
      ref.current.enterFullscreenControlingMode();
    }
  }, []);

  return (
    <div tabIndex={0} style={{ display: 'flex', flexDirection: 'column', width: '100%', height: '100%', outline: 'none' }}>
      <div style={{ display: 'flex', height: '30px' }}>
        <button onClick={enterFullscreen}>Play</button>
        <code>pos: {JSON.stringify(pos, null, 2)}</code>
      </div>
      <game-view ref={ref} style={{ flexGrow: '1', width: '100%' }}></game-view>
    </div>
  );
});

@injectable()
export class FxdkGameView extends ReactWidget {
  static readonly ID = 'fxdkGameView';

  @postConstruct()
  init(): void {
    this.id = FxdkGameView.ID;
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
