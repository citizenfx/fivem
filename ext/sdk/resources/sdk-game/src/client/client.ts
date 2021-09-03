import '@citizenfx/client';
import './connectedSender';
import { GameMode, getGameMode } from '../shared';

console.log('sdk-game:client started!');

switch (getGameMode()) {
  case GameMode.NORMAL: {
    require('./sdk-support-mode/sdk-support-mode');
    break;
  }

  case GameMode.WORLD_EDITOR: {
    require('../world-editor/world-editor-client');
    break;
  }
}
