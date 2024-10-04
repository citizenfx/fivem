import { injectable } from "inversify";
import { disposableFromFunction, IDisposableObject } from "fxdk/base/disposable";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { Ticker } from "backend/ticker";
import { ServerUpdateChannel } from "shared/api.types";

export interface ServerStartRequest {
  fxserverCwd: string,
  updateChannel: ServerUpdateChannel,

  // Will force game server to not load blank.cfg but use this list instead
  cmdList?: string[],

  licenseKey?: string,
  steamWebApiKey?: string,
  tebexSecret?: string,
}

export interface ServerResourceDescriptor {
  name: string,
  path: string,
}

export interface ServerVariableDescriptor {
  name: string,
  setter: string,
  value: string,
}

@injectable()
export class GameServerRuntime {
  protected serverChangesTicker = new Ticker();

  protected readonly setResourcesEvent = new SingleEventEmitter<ServerResourceDescriptor[]>();
  protected serverResources: ServerResourceDescriptor[] = [];
  onSetResources(cb: (resources: ServerResourceDescriptor[]) => void): IDisposableObject {
    return disposableFromFunction(this.setResourcesEvent.addListener(cb));
  }
  setResources(resources: ServerResourceDescriptor[]) {
    this.serverResources = resources;

    this.propagateChanges();
  }
  getResources(): ServerResourceDescriptor[] {
    return this.serverResources;
  }

  protected readonly setVariablesEvent = new SingleEventEmitter<ServerVariableDescriptor[]>();
  protected serverVariables: ServerVariableDescriptor[] = [];
  onSetVariables(cb: (variables: ServerVariableDescriptor[]) => void): IDisposableObject {
    return disposableFromFunction(this.setVariablesEvent.addListener(cb));
  }
  setVariables(variables: ServerVariableDescriptor[]) {
    this.serverVariables = variables;

    this.propagateChanges();
  }
  getVariables(): ServerVariableDescriptor[] {
    return this.serverVariables;
  }

  protected readonly startResourceEvent = new SingleEventEmitter<string>();
  onStartResource(cb: (resourceName: string) => void): IDisposableObject {
    return disposableFromFunction(this.startResourceEvent.addListener(cb));
  }
  startResource(resourceName: string) {
    this.startResourceEvent.emit(resourceName);
  }

  protected readonly stopResourceEvent = new SingleEventEmitter<string>();
  onStopResource(cb: (resourceName: string) => void): IDisposableObject {
    return disposableFromFunction(this.stopResourceEvent.addListener(cb));
  }
  stopResource(resourceName: string) {
    this.stopResourceEvent.emit(resourceName);
  }

  protected readonly restartResourceEvent = new SingleEventEmitter<string>();
  onRestartResource(cb: (resourceName: string) => void): IDisposableObject {
    return disposableFromFunction(this.restartResourceEvent.addListener(cb));
  }
  restartResource(resourceName: string) {
    this.restartResourceEvent.emit(resourceName);
  }

  protected readonly reloadResourceEvent = new SingleEventEmitter<string>();
  onReloadResource(cb: (resourceName: string) => void): IDisposableObject {
    return disposableFromFunction(this.reloadResourceEvent.addListener(cb));
  }
  reloadResource(resourceName: string) {
    this.reloadResourceEvent.emit(resourceName);
  }

  protected readonly sendCommandEvent = new SingleEventEmitter<string>();
  onSendCommand(cb: (cmd: string) => void): IDisposableObject {
    return disposableFromFunction(this.sendCommandEvent.addListener(cb));
  }
  sendCommand(cmd: string) {
    this.sendCommandEvent.emit(cmd);
  }

  protected readonly requestResourcesStateEvent = new SingleEventEmitter<void>();
  onRequestResourcesState(cb: () => void) {
    return disposableFromFunction(this.requestResourcesStateEvent.addListener(cb));
  }
  requestResourcesState() {
    this.requestResourcesStateEvent.emit();
  }

  private propagateChanges() {
    this.serverChangesTicker.whenTickEnds(() => {
      this.setVariablesEvent.emit(this.serverVariables);
      this.setResourcesEvent.emit(this.serverResources);
    });
  }
}
