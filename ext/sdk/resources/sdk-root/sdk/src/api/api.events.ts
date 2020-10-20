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
}

export const systemEvents = {
  on(eventType: SystemEvent, cb: any) {
    console.log('[SYSTEM_EVENTS] on', eventType, cb);
    systemEventEmitter.on(eventType + '', cb);
  },
  emit(eventType: SystemEvent, data?: any) {
    console.log('[SYSTEM_EVENTS] emit', eventType, data);
    systemEventEmitter.emit(eventType + '', data);
  }
};
