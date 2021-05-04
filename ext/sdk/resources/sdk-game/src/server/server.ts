import '@citizenfx/server';
import { GameMode, getGameMode, getServerMode, ServerMode } from '../shared';

switch (getServerMode()) {
  case ServerMode.NORMAL: {
    console.log('^3Hooray, FxDK server mode!');

    if (getGameMode() === GameMode.WORLD_EDITOR) {
      require('../world-editor/world-editor-server');
    }
    break;
  }

  case ServerMode.LEGACY: {
    require('./legacy-server-mode/legacy-server-mode');
    break;
  }
}
