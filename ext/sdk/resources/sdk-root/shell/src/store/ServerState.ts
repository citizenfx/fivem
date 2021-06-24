import { makeAutoObservable } from "mobx";
import { projectApi, serverApi } from "shared/api.events";
import { ProjectStartServerRequest } from "shared/api.requests";
import { ServerStates, ServerUpdateChannel, ServerUpdateStates } from "shared/api.types";
import { onApiMessage, sendApiMessage } from "utils/api";
import { getProjectTebexSecretVar } from "utils/projectStorage";
import { sendCommandToGameClient } from "utils/sendCommand";
import { onWindowEvent } from "utils/windowMessages";
import { ProjectState } from "./ProjectState";

export const ServerState = new class ServerState {
  constructor() {
    makeAutoObservable(this);

    onApiMessage(serverApi.state, this.setState);
    onApiMessage(serverApi.updateChannelsState, this.setUpdateChannelsState);
    onApiMessage(serverApi.resourcesState, this.setResourcesState);

    onWindowEvent('server:sendCommand', (cmd: string) => {
      this.sendCommand(cmd);
    });

    onWindowEvent('fxdk:startServer', () => {
      this.startServer();
    });
  }

  public state: ServerStates | null = null;

  public updateChannelsState = Object.create(null);

  public clientConnected = false;

  public resourcesState = Object.create(null);

  public ack() {
    sendApiMessage(serverApi.ackState);
    sendApiMessage(serverApi.ackUpdateChannelsState);

    sendCommandToGameClient('sdk:ackConnected');
  }

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
    if (ProjectState.hasProject) {
      const project = ProjectState.project;

      const steamWebApiKey = ''; //getProjectSteamWebApiKeyVar(project);
      const tebexSecret = getProjectTebexSecretVar(project);

      sendApiMessage(projectApi.startServer, {
        steamWebApiKey,
        tebexSecret,
      } as ProjectStartServerRequest);
    }
  }

  stopServer() {
    sendApiMessage(projectApi.stopServer);
  }

  toggleServer() {
    if (this.isUp) {
      this.stopServer();
    }
    if (this.isDown) {
      this.startServer();
    }
  }

  sendCommand(cmd: string) {
    sendApiMessage(serverApi.sendCommand, cmd);
  }

  checkForUpdates(updateChannel: ServerUpdateChannel) {
    sendApiMessage(serverApi.checkForUpdates, updateChannel);
  }

  installUpdate(updateChannel: ServerUpdateChannel) {
    sendApiMessage(serverApi.checkForUpdates, updateChannel);
  }

  private setState = (state: ServerStates) => {
    if (state === ServerStates.up) {
      sendApiMessage(serverApi.ackResourcesState);
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
};
