import { DisposableObject } from "backend/disposable-container";
import { fastRandomId } from "utils/random";

type RawEventFromDirectory = string;
type RawEventFromFile = string;
type RawEventToDirectory = string;
type RawEventToFile = string;
type RawEvent = [
  FsWatcherEventType,
  RawEventFromDirectory,
  RawEventFromFile,
  RawEventToDirectory,
  RawEventToFile,
];

const eventListeners: Record<number, (events: RawEvent[]) => void> = Object.create(null);
const errorListeners: Record<number, (error: string) => void> = Object.create(null);

on('sdk:fileWatcherEvents', (watcherId: number, events: RawEvent[]) => {
  const listener = eventListeners[watcherId];
  if (!listener) {
    return;
  }

  listener(events);
});

on('sdk:fileWatcherError', (watcherId: number, error: string) => {
  const listener = errorListeners[watcherId];
  if (!listener) {
    return;
  }

  listener(error);
});

export enum FsWatcherEventType {
  CREATED = 0,
  DELETED = 1,
  MODIFIED = 2,
  RENAMED = 3,
}

export interface FsWatcherEvent {
  action: FsWatcherEventType,

  entryPath: string,

  /**
   * For RENAMED event
   */
  oldEntryPath?: string,
}

export interface FsWatcherOptions {
  path: string,
  ignoredPaths: string[],

  onCreated?: (entryPath: string) => void,
  onDeleted?: (entryPath: string) => void,
  onModified?: (entryPath: string) => void,
  onRenamed?: (entryPath: string, oldEntryPath: string) => void,

  logger?: (...args: any[]) => void,
}

export class FsWatcher implements DisposableObject {
  private watcherId: number = -1;
  private ignoredPaths: string[];
  private logger: (...args: any[]) => void;

  protected onCreated = (entryPath: string) => this.logger('Created!', entryPath);
  protected onDeleted = (entryPath: string) => this.logger('Deleted!', entryPath);
  protected onModified = (entryPath: string) => this.logger('Modified!', entryPath);
  protected onRenamed = (entryPath: string, oldEntryPath: string) => this.logger('Renamed! From', oldEntryPath, 'to', entryPath);

  constructor(options: FsWatcherOptions) {
    this.ignoredPaths = options.ignoredPaths;
    this.logger = options.logger || ((...args) => console.log(...args));

    this.onCreated = options.onCreated || ((entryPath: string) => this.logger('Created!', entryPath));
    this.onDeleted = options.onDeleted || ((entryPath: string) => this.logger('Deleted!', entryPath));
    this.onModified = options.onModified || ((entryPath: string) => this.logger('Modified!', entryPath));
    this.onRenamed = options.onRenamed || ((entryPath: string, oldEntryPath: string) => this.logger('Renamed! From', oldEntryPath, 'to', entryPath));

    const requestId = fastRandomId();

    const setWatcherId = (requestIdCandidate: string, watcherId: number) => {
      if (requestId !== requestIdCandidate) {
        return;
      }

      eventListeners[watcherId] = this.handleEvents;
      errorListeners[watcherId] = this.handleError;

      RemoveEventHandler('sdk:fileWatcherId', setWatcherId);

      this.watcherId = watcherId;
    };

    on('sdk:fileWatcherId', setWatcherId);
    emit('sdk:startFileWatcher', options.path, requestId);
  }

  dispose() {
    emit('sdk:stopFileWatcher', this.watcherId);

    delete eventListeners[this.watcherId];
    delete errorListeners[this.watcherId];
  }

  private handleEvents = (rawEvents: RawEvent[]) => {
    rawEvents.forEach(([action, fromDirectory, fromFile, toDirectory, toFile]: RawEvent) => {
      const fromEntryPath = `${fromDirectory}\\${fromFile}`;
      if (this.ignoredPaths.some((ignoredPath) => fromEntryPath.indexOf(ignoredPath) > -1)) {
        return;
      }

      const toEntryPath = `${toDirectory}\\${toFile}`;

      switch (action) {
        case FsWatcherEventType.CREATED: {
          return this.onCreated(fromEntryPath);
        }
        case FsWatcherEventType.DELETED: {
          return this.onDeleted(fromEntryPath);
        }
        case FsWatcherEventType.MODIFIED: {
          return this.onModified(fromEntryPath);
        }
        case FsWatcherEventType.RENAMED: {
          return this.onRenamed(toEntryPath, fromEntryPath);
        }
      }
    });
  };

  private handleError = (error: string) => {
    this.logger('WATCHER ERROR!!! ', error);
  };
}
