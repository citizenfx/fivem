import { AppContribution } from "backend/app/app-contribution";
import { GameServerService } from "backend/game-server/game-server-service";
import { LogService } from "backend/logger/log-service";
import { inject, injectable } from "inversify";
import { ServerStates } from "shared/api.types";
import { GameStates } from "./game-contants";
import { GameService } from "./game-service";

@injectable()
export class GameConnectionService implements AppContribution {
  @inject(LogService)
  protected readonly logService: LogService;

  @inject(GameService)
  protected readonly gameService: GameService;

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  private gameFPSLimited = false;

  boot() {
    this.gameService.onGameStateChange((gameState) => {
      this.processState(gameState, this.gameServerService.getState());
    });

    this.gameServerService.onServerStateChange((serverState) => {
      this.processState(this.gameService.getGameState(), serverState);
    });
  }

  private processState(gameState: GameStates, serverState: ServerStates) {
    switch (true) {
      case gameState === GameStates.READY && serverState === ServerStates.up: {
        this.logService.log('[GameConnectionService] Connecting client to server')
        return emit('sdk:connectClientTo', '127.0.0.1:30120');
      }

      case gameState === GameStates.CONNECTED && serverState === ServerStates.up: {
        this.logService.log('[GameConnectionService] FPS limit lifted');
        return this.unlimitFPS();
      }

      case gameState === GameStates.UNLOADING && serverState === ServerStates.down: {
        this.logService.log('[GameConnectionService] Disconnecting client from server');
        return emit('sdk:disconnectClient');
      }

      default: {
        this.logService.log('[GameConnectionService] FPS has been limited to 60');
        return this.limitFPS();
      }
    }
  }

  private limitFPS() {
    if (this.gameFPSLimited) {
      return;
    }

    this.gameFPSLimited = true;
    emit('sdk:setFPSLimit', 60);
  }

  private unlimitFPS() {
    if (!this.gameFPSLimited) {
      return;
    }

    this.gameFPSLimited = false;
    emit('sdk:setFPSLimit', 0);
  }
}
