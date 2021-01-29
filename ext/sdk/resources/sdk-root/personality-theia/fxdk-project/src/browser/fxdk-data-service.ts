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

@injectable()
export class FxdkDataService {
  public data: { [key: string]: any } = {};

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

  sendMessageToShell(type: string, data?: any) {
    window.parent.postMessage({ type, data }, '*');
  }
}
