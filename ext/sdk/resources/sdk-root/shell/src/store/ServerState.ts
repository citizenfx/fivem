import { Api } from "fxdk/browser/Api";
import { ShellLifecycle, ShellLifecyclePhase } from "fxdk/browser/shellLifecycle";
import { Project } from "fxdk/project/browser/state/project";
import { ProjectLoader } from "fxdk/project/browser/state/projectLoader";
import { ProjectApi } from "fxdk/project/common/project.api";
import { makeAutoObservable } from "mobx";
import { serverApi } from "shared/api.events";
import { APIRQ } from "shared/api.requests";
import { ServerStates, ServerUpdateChannel, ServerUpdateStates } from "shared/api.types";
import { ShellEvents } from "shell-api/events";
import { sendCommandToGameClient } from "utils/sendCommand";

export const ServerState = new class ServerState {
  constructor() {
    makeAutoObservable(this);

    ShellLifecycle.onPhase(ShellLifecyclePhase.Booting, () => sendCommandToGameClient('sdk:ackConnected'));

    Api.on(serverApi.state, this.setState);
    Api.on(serverApi.updateChannelsState, this.setUpdateChannelsState);
    Api.on(serverApi.resourcesState, this.setResourcesState);

    ShellEvents.on('server:sendCommand', this.sendCommand.bind(this));
    ShellEvents.on('fxdk:startServer', this.startServer.bind(this));
  }

  public state: ServerStates | null = null;

  public updateChannelsState = Object.create(null);

  public clientConnected = false;

  public resourcesState = Object.create(null);

  isResourceRunning(resourceName: string): boolean {
    return !!this.resourcesState[resourceName];
  }

  get isUp(): boolean {
    return this.state === ServerStates.up;
  }

  get isDown(): boolean {
    return this.state === ServerStates.down;
  }

  get isBooting(): boolean {
    return this.state === ServerStates.booting;
  }

  getUpdateChannelState(updateChannel: ServerUpdateChannel | null): ServerUpdateStates | null {
    if (updateChannel === null) {
      return null;
    }

    return this.updateChannelsState[updateChannel];
  }

  setClientConnected(connected: boolean) {
    this.clientConnected = connected;
  }

  startServer() {
    if (this.state !== ServerStates.down) {
      return;
    }

    if (ProjectLoader.hasProject) {
      const steamWebApiKey = ''; //Project.localStorage.steamWebApiKey;
      const tebexSecret = Project.localStorage.tebexSecret;

      Api.send(ProjectApi.ServerEndpoints.start, {
        steamWebApiKey,
        tebexSecret,
      } as APIRQ.ProjectStartServer);
    }
  }

  stopServer() {
    Api.send(ProjectApi.ServerEndpoints.stop);
  }

  toggleServer() {
    if (this.isUp) {
      this.stopServer();
    }
    if (this.isDown) {
      this.startServer();
    }
  }

  readonly stopResource = (name: string) => Api.send(serverApi.stopResource, name);
  readonly startResource = (name: string) => Api.send(serverApi.startResource, name);
  readonly restartResource = (name: string) => Api.send(serverApi.restartResource, name);

  sendCommand(cmd: string) {
    Api.send(serverApi.sendCommand, cmd);
  }

  checkForUpdates(updateChannel: ServerUpdateChannel) {
    Api.send(serverApi.checkForUpdates, updateChannel);
  }

  installUpdate(updateChannel: ServerUpdateChannel) {
    Api.send(serverApi.checkForUpdates, updateChannel);
  }

  private setState = (state: ServerStates) => {
    if (state === ServerStates.up) {
      Api.send(serverApi.ackResourcesState);
    } else if (state === ServerStates.down) {
      this.resourcesState = Object.create(null);
    }

    this.state = state;
  };

  private setUpdateChannelsState = (state) => {
    this.updateChannelsState = state;
  };

  private setResourcesState = (state) => {
    this.resourcesState = state;
  };
}();
