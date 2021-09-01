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

type EntryPath = string;
type OldEntryPath = string;
export type FsWatcherEvent = [FsWatcherEventType, EntryPath, OldEntryPath?];

export interface FsWatcherOptions {
  path: string,
  ignoredPaths: string[],

  onEvent?(event: FsWatcherEvent): void,

  logger?: (...args: any[]) => void,
}

export class FsWatcher implements DisposableObject {
  private watcherId: number = -1;
  private ignoredPaths: string[];
  private logger: (...args: any[]) => void;

  protected onEvent = (event: FsWatcherEvent) => this.logger('Event!', event);

  constructor(options: FsWatcherOptions) {
    this.ignoredPaths = options.ignoredPaths;
    this.logger = options.logger || ((...args) => console.log(...args));

    if (options.onEvent) {
      this.onEvent = options.onEvent;
    }

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
    const eventForPath: Record<string, [FsWatcherEventType, string, string?] | null> = Object.create(null);

    rawEvents.forEach(([action, fromDirectory, fromFile, toDirectory, toFile]) => {
      const fromEntryPath = `${fromDirectory}\\${fromFile}`;
      const toEntryPath = `${toDirectory}\\${toFile}`;

      // Untill we have "complete" git integration - don't care about that
      if (fromEntryPath.indexOf('\\.git\\') > -1 || fromFile === '.git') {
        return;
      }

      if (this.ignoredPaths.some((ignoredPath) => fromEntryPath.indexOf(ignoredPath) > -1)) {
        return;
      }

      if (action === FsWatcherEventType.RENAMED) {
        eventForPath[fromEntryPath] = null;
        eventForPath[toEntryPath] = [action, toEntryPath, fromEntryPath];
        return;
      }

      const event = eventForPath[fromEntryPath];

      // This path is ignored
      if (event === null) {
        return;
      }

      if (!event) {
        eventForPath[fromEntryPath] = [action, fromEntryPath];
        return;
      }

      // CREATED -> [] -> DELETED in one batch - why bother then
      if (action === FsWatcherEventType.DELETED && event[0] === FsWatcherEventType.CREATED) {
        eventForPath[fromEntryPath] = null;
        return;
      }
    });

    Object.values(eventForPath)
      .filter(Boolean)
      .sort((a, b) => a![1].length - b![1].length)
      .forEach((event) => this.onEvent(event!));
  };

  private handleError = (error: string) => {
    this.logger('WATCHER ERROR!!! ', error);
  };
}
