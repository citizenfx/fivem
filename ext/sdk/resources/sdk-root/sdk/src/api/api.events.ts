import { EventEmitter } from 'events';

const systemEventEmitter = new EventEmitter();

export enum SystemEvent {
  assetCreateRequest,
  assetCreated,
  assetRenameRequest,
  assetRenamed,
  assetDeleteRequest,
  assetDeleted,

  relinkResources,
  restartResource,

  forceStopServer,

  setStatus,
}

export const systemEvents = {
  on(eventType: SystemEvent, cb: any) {
    systemEventEmitter.on(eventType + '', cb);
  },
  emit(eventType: SystemEvent, data?: any) {
    systemEventEmitter.emit(eventType + '', data);
  }
};
