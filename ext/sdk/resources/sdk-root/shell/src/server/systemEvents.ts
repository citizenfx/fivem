import { EventEmitter } from 'events';

const systemEventEmitter = new EventEmitter();

export enum SystemEvent {
  assetCreateRequest,
  assetCreated,
  assetRenameRequest,
  assetRenamed,
  assetDeleteRequest,
  assetDeleted,

  createNotification,

  refreshResources,
  relinkResources,
  restartResource,

  forceStopServer,
}

export const systemEvents = {
  on(eventType: SystemEvent, cb: any) {
    systemEventEmitter.on(eventType + '', cb);

    return () => systemEventEmitter.off(eventType + '', cb);
  },
  emit(eventType: SystemEvent, data?: any) {
    systemEventEmitter.emit(eventType + '', data);
  }
};
