import { Disposable } from '@theia/core';
import { injectable } from 'inversify';


export class SingleEventEmitter<T> {
  private listeners: Set<(arg: T) => void> = new Set();

  on(listener: (arg: T) => void): Disposable {
    this.listeners.add(listener);

    return Disposable.create(() => this.listeners.delete(listener));
  }

  emit(data: T) {
    this.listeners.forEach((listener) => listener(data));
  }
}

export interface StructuredMessage {
  channel: string;
  message: string;
}

// resourceDatas.emplace_back(resource->GetName(), avgTickMs, avgFrameFraction, memorySize, streamingSize);
export type ResourceName = string;
export type ResourceAvgTickMs = number;
export type ResourceAvgFrameFraction = number;
export type ResourceMemorySize = number;
export type ResourceStreamingSize = number;
export type ClientResourceData = [ResourceName, ResourceAvgTickMs, ResourceAvgFrameFraction, ResourceMemorySize, ResourceStreamingSize];
export type ServerResourceData = ClientResourceData;

@injectable()
export class FxdkDataService {
  public data: { [key: string]: any } = {};

  private clientResourcesData: ClientResourceData[] = [];
  private serverResourcesData: ServerResourceData[] = [];

  private bufferedServerOutput: string = '';
  private structuredServerMessages: StructuredMessage[] = [];

  private readonly bufferedServerOutputChanged = new SingleEventEmitter<string>();
  public onBufferedServerOutputChanged(cb: (output: string) => void): Disposable {
    return this.bufferedServerOutputChanged.on(cb);
  }

  private readonly structuredServerMessageReceivedEvent = new SingleEventEmitter<StructuredMessage>();
  public onStructuredServerMessageReceived(cb: (st: StructuredMessage) => void): Disposable {
    return this.structuredServerMessageReceivedEvent.on(cb);
  }

  private readonly clearAllServerOutputsEvent = new SingleEventEmitter<void>();
  public onClearAllServerOutputs(cb: () => void): Disposable {
    return this.clearAllServerOutputsEvent.on(cb);
  }

  //#region client-resources-data
  private readonly clientResourcesDataEvent = new SingleEventEmitter<ClientResourceData[]>();
  public onClientResourcesData(cb: (data: ClientResourceData[]) => void): Disposable {
    return this.clientResourcesDataEvent.on(cb);
  }

  getClientResourcesData(): ClientResourceData[] {
    return this.clientResourcesData;
  }

  setClientResourcesData(data: ClientResourceData[]) {
    this.clientResourcesData = data;

    this.clientResourcesDataEvent.emit(data);
  }
  //#endregion

  //#region server-resources-data
  private readonly serverResourcesDataEvent = new SingleEventEmitter<ServerResourceData[]>();
  public onServerResourcesData(cb: (data: ServerResourceData[]) => void): Disposable {
    return this.serverResourcesDataEvent.on(cb);
  }

  getServerResourcesData(): ServerResourceData[] {
    return this.serverResourcesData;
  }

  setServerResourcesData(data: ServerResourceData[]) {
    this.serverResourcesData = data;

    this.serverResourcesDataEvent.emit(data);
  }
  //#endregion

  getBufferedServerOutput(): string {
    return this.bufferedServerOutput;
  }

  getStructuredServerMessages(): StructuredMessage[] {
    return this.structuredServerMessages;
  }

  clearAllServerOutputs() {
    this.bufferedServerOutput = '';
    this.structuredServerMessages = [];

    this.clearAllServerOutputsEvent.emit();
  }

  setBufferedServerOutput(output: string) {
    this.bufferedServerOutput = output;
    this.bufferedServerOutputChanged.emit(this.bufferedServerOutput);
  }

  receiveStructuredServerMessage(message: StructuredMessage) {
    this.structuredServerMessages.push(message);
    this.structuredServerMessageReceivedEvent.emit(message);
  }

  private structuredGameMessages: StructuredMessage[] = [];
  private readonly structuredGameMessageReceivedEvent = new SingleEventEmitter<StructuredMessage>();
  public onStructuredGameMessageReceived(cb: (st: StructuredMessage) => void): Disposable {
    return this.structuredGameMessageReceivedEvent.on(cb);
  }

  private readonly clearGameOutputEvent = new SingleEventEmitter<void>();
  public onClearGameOutput(cb: () => void): Disposable {
    return this.clearGameOutputEvent.on(cb);
  }

  getStructuredGameMessage(): StructuredMessage[] {
    return this.structuredGameMessages;
  }

  receiveStructuredGameMessage(message: StructuredMessage) {
    this.structuredGameMessages.push(message);
    this.structuredGameMessageReceivedEvent.emit(message);
  }

  clearGameOutput() {
    this.structuredGameMessages = [];
    this.clearGameOutputEvent.emit();
  }

  sendMessageToShell(type: string, data?: any) {
    window.parent.postMessage({ type, data }, '*');
  }

  private theiaIsActive = false;
  private theiaIsActiveEvent = new SingleEventEmitter<boolean>();
  onTheiaIsActiveChange(cb: (isActive: boolean) => void): Disposable {
    return this.theiaIsActiveEvent.on(cb);
  }
  setTheiaIsActive(isActive: boolean) {
    this.theiaIsActive = isActive;
    this.theiaIsActiveEvent.emit(isActive);
  }
  getTheiaIsActive(): boolean {
    return this.theiaIsActive;
  }
}
