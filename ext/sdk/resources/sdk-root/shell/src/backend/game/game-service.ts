import { ApiClient } from "backend/api/api-client";
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from "backend/api/api-decorators";
import { AppContribution } from "backend/app/app-contribution";
import { Disposable, disposableFromFunction } from "backend/disposable-container";
import { LogService } from "backend/logger/log-service";
import { NotificationService } from "backend/notification/notification-service";
import { inject, injectable } from "inversify";
import { gameApi } from "shared/api.events";
import { NetLibraryConnectionState, SDKGameProcessState } from "shared/native.enums";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { GameStates } from "./game-contants";

@injectable()
export class GameService implements ApiContribution, AppContribution {
  getId(): string {
    return 'GameService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  private gameState = GameStates.NOT_RUNNING;

  private gameBuildNumber = 0;

  private gameLaunched = false;

  private gameUnloaded = true;

  private gameProcessState = SDKGameProcessState.GP_STOPPED;

  private connectionState = NetLibraryConnectionState.CS_IDLE;

  private restartPending = false;

  private readonly gameStateChangeEvent = new SingleEventEmitter<GameStates>();
  onGameStateChange(cb: (gameState: GameStates) => void): Disposable {
    return this.gameStateChangeEvent.addListener(cb);
  }

  getBuildNumber(): number {
    return this.gameBuildNumber;
  }

  getGameState(): GameStates {
    return this.gameState;
  }

  async boot() {
    const gameBuildNumberPromise = new Promise<void>((resolve) => {
      const handler = (gameBuildNumber: number) => {
        this.gameBuildNumber = gameBuildNumber;

        RemoveEventHandler('sdk:setBuildNumber', handler);

        resolve();
      };

      on('sdk:setBuildNumber', handler);
      emit('sdk:getBuildNumber');
    });

    on('sdk:gameLaunched', () => {
      this.gameLaunched = true;
      this.gameUnloaded = true;
      this.toGameState(GameStates.READY);

      this.apiClient.emit(gameApi.gameLaunched, true);
    });

    on('sdk:connectionStateChanged', (current: NetLibraryConnectionState, previous: NetLibraryConnectionState) => {
      this.connectionState = current;

      if (this.gameLaunched) {
        if (current === NetLibraryConnectionState.CS_IDLE) {
          this.toGameState(GameStates.UNLOADING);
        } else if (this.gameUnloaded) {
          this.gameUnloaded = false;
          this.toGameState(GameStates.LOADING);
        }
      }

      this.apiClient.emit(gameApi.connectionStateChanged, { current, previous });
    });

    on('sdk:gameProcessStateChanged', (current: SDKGameProcessState, previous: SDKGameProcessState) => {
      this.gameProcessState = current;

      if (this.gameLaunched && (current === SDKGameProcessState.GP_STOPPED || current === SDKGameProcessState.GP_STOPPING)) {
        const previousConnectionState = this.connectionState;

        this.gameLaunched = false;
        this.connectionState = NetLibraryConnectionState.CS_IDLE;
        this.toGameState(GameStates.NOT_RUNNING);

        this.apiClient.emit(gameApi.connectionStateChanged, { current: this.connectionState, previous: previousConnectionState });
        this.apiClient.emit(gameApi.gameLaunched, this.gameLaunched);
      }

      if (current === SDKGameProcessState.GP_STOPPED) {
        if (this.restartPending) {
          this.restartPending = false;
        } else {
          this.notificationService.error('It looks like game has crashed, restarting it now', 5000);
          this.startGame();
        }
      }

      this.apiClient.emit(gameApi.gameProcessStateChanged, { current, previous });
    });

    on('sdk:gameUnloaded', () => {
      this.gameUnloaded = true;

      if (this.gameLaunched) {
        this.toGameState(GameStates.READY);
      }
    });

    on('sdk:backendMessage', (message: string) => {
      if (this.gameLaunched && message === JSON.stringify({ type: 'connected' })) {
        this.toGameState(GameStates.CONNECTED);
      }
    });

    this.startGame();

    await gameBuildNumberPromise;
  }

  beginUnloading() {
    this.toGameState(GameStates.UNLOADING);
  }

  @handlesClientEvent(gameApi.ack)
  ack() {
    this.apiClient.emit(gameApi.ack, {
      gameState: this.gameState,
      gameLaunched: this.gameLaunched,
      gameProcessState: this.gameProcessState,
      connectionState: this.connectionState,
    });
  }

  @handlesClientEvent(gameApi.start)
  startGame() {
    emit('sdk:startGame');
  }

  @handlesClientEvent(gameApi.stop)
  stopGame() {
    this.gameLaunched = false;
    this.gameUnloaded = true;

    this.toGameState(GameStates.NOT_RUNNING);

    emit('sdk:stopGame');
  }

  @handlesClientEvent(gameApi.restart)
  restartGame() {
    this.restartPending = true;
    this.gameLaunched = false;
    this.gameUnloaded = true;

    this.toGameState(GameStates.NOT_RUNNING);

    emit('sdk:restartGame');
  }

  private toGameState(newState: GameStates) {
    if (newState === this.gameState) {
      return;
    }

    this.gameState = newState;
    this.gameStateChangeEvent.emit(this.gameState);
    this.apiClient.emit(gameApi.gameState, this.gameState);
  }
}
