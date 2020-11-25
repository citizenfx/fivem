import { ApiClient, ServerDataStates, States } from "shared/api.types";
import { stateApi } from "shared/api.events";


export class StateApi {
  state: States = States.booting;
  serverDataState: ServerDataStates = ServerDataStates.checking;

  gameLaunched: boolean = false;

  constructor(
    private readonly client: ApiClient,
  ) {
    on('sdk:gameLaunched', this.onGameLaunched);

    this.client.on(stateApi.ackState, this.onAckState);
  }

  onGameLaunched = () => {
    this.gameLaunched = true;
    this.client.emit(stateApi.gameLaunched);
  };

  onAckState = () => {
    this.ackState();

    if (this.gameLaunched) {
      this.client.emit(stateApi.gameLaunched);
    }
  };

  ackState() {
    this.client.emit(stateApi.state, this.state);
  }

  ackServerDataState() {
    this.client.emit(stateApi.serverDataState, this.serverDataState);
  }

  ackFxserverDownloadState(total: number, downloaded: number) {
    this.client.emit(stateApi.fxserverDownload, {
      total,
      downloaded,
    });
  }

  ackFxserverUnpackState(total: number, downloaded: number) {
    this.client.emit(stateApi.fxserverUnpack, {
      total,
      downloaded,
    });
  }

  toState(newState: States) {
    this.state = newState;
    this.ackState();
  }

  toServerDataState(newServerDataState: ServerDataStates) {
    this.serverDataState = newServerDataState;
    this.ackServerDataState();
  }
}
