import { inject, injectable } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { stateApi } from "shared/api.events";
import { ServerDataStates, AppStates } from "shared/api.types";
import { ApiContribution } from "backend/api/api.extensions";
import { AppContribution } from "./app.extensions";
import { LogService } from "backend/logger/log-service";

@injectable()
export class AppStateService implements ApiContribution, AppContribution {
  getId() {
    return 'AppStateService';
  }

  public readonly onStateChange = new SingleEventEmitter<AppStates>();

  public readonly onServerDataStateChange = new SingleEventEmitter<ServerDataStates>();

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;


  private userId: string = '';

  protected state: AppStates = AppStates.booting;
  protected serverDataState: ServerDataStates = ServerDataStates.checking;

  protected gameLaunched: boolean = false;

  async boot() {
    this.apiClient.onClientConnected.addListener(() => this.onAckState());

    on('sdk:gameLaunched', () => {
      this.gameLaunched = true;
      this.apiClient.emit(stateApi.gameLaunched);
    });

    this.userId = await this.fetchUserId();

    this.logService.setUserId(this.userId);
  }

  getUserId(): string {
    return this.userId;
  }

  isGameLaunched() {
    return this.gameLaunched;
  }

  getState() {
    return this.state;
  }

  getServerDataState() {
    return this.serverDataState;
  }

  onAckState() {
    this.ackState();

    this.apiClient.emit(stateApi.setUserId, this.userId);

    if (this.gameLaunched) {
      this.apiClient.emit(stateApi.gameLaunched);
    }
  };

  ackState() {
    this.apiClient.emit(stateApi.state, this.state);
  }

  ackServerDataState() {
    this.apiClient.emit(stateApi.serverDataState, this.serverDataState);
  }

  toState(newState: AppStates) {
    this.state = newState;
    this.onStateChange.emit(newState);
    this.ackState();
  }

  toServerDataState(newServerDataState: ServerDataStates) {
    this.serverDataState = newServerDataState;
    this.onServerDataStateChange.emit(newServerDataState);
    this.ackServerDataState();
  }

  private fetchUserId(): Promise<string> {
    return new Promise((resolve) => {
      emit('sdk:getUserId');

      const handler = (id: string) => {
        resolve(id);
        RemoveEventHandler('sdk:setUserId', handler);
      };

      on('sdk:setUserId', handler);
    });
  }
}
