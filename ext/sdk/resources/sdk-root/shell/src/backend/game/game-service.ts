import { ApiClient } from "backend/api/api-client";
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from "backend/api/api-decorators";
import { AppContribution } from "backend/app/app-contribution";
import { NotificationService } from "backend/notification/notification-service";
import { inject, injectable } from "inversify";
import { gameApi } from "shared/api.events";
import { NetLibraryConnectionState, SDKGameProcessState } from "shared/native.enums";

@injectable()
export class GameService implements ApiContribution, AppContribution {
  getId(): string {
    return 'GameService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  private gameBuildNumber: number = 0;

  private gameLaunched: boolean = false;

  private gameProcessState: SDKGameProcessState = SDKGameProcessState.GP_STOPPED;

  private connectionState: NetLibraryConnectionState = NetLibraryConnectionState.CS_IDLE;

  getBuildNumber(): number {
    return this.gameBuildNumber;
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

      this.apiClient.emit(gameApi.gameLaunched, true);
    });

    on('sdk:connectionStateChanged', (current: NetLibraryConnectionState, previous: NetLibraryConnectionState) => {
      this.connectionState = current;

      this.apiClient.emit(gameApi.connectionStateChanged, { current, previous });
    });

    on('sdk:gameProcessStateChanged', (current: SDKGameProcessState, previous: SDKGameProcessState) => {
      this.gameProcessState = current;

      if (this.gameLaunched && (current === SDKGameProcessState.GP_STOPPED || current === SDKGameProcessState.GP_STOPPING)) {
        const previousConnectionState = this.connectionState;

        this.gameLaunched = false;
        this.connectionState = NetLibraryConnectionState.CS_IDLE;

        this.apiClient.emit(gameApi.connectionStateChanged, { current: this.connectionState, previous: previousConnectionState });
        this.apiClient.emit(gameApi.gameLaunched, this.gameLaunched);
      }

      if (current === SDKGameProcessState.GP_STOPPED) {
        this.notificationService.error('It looks like game has crashed, restarting it now', 5000);
        this.startGame();
      }

      this.apiClient.emit(gameApi.gameProcessStateChanged, { current, previous });
    });

    this.startGame();

    await gameBuildNumberPromise;
  }

  @handlesClientEvent(gameApi.ack)
  ack() {
    this.apiClient.emit(gameApi.ack, {
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
    emit('sdk:stopGame');
  }

  @handlesClientEvent(gameApi.restart)
  restartGame() {
    emit('sdk:restartGame');
  }
}
