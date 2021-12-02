import * as path from 'path';
import { IDisposableObject } from "fxdk/base/disposable";
import { fastRandomId } from "utils/random";

function normalizeRawEvent([action, fromDir, fromFile, toDir, toFile]: RawEvent): FsWatcherEvent {
  return [action, path.join(fromDir, fromFile), (toDir && toFile) && path.join(toDir, toFile)];
}

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

export function createFsWatcherEvent(type: FsWatcherEventType, entryPath: string, oldEntryPath?: string): FsWatcherEvent {
  return [type, entryPath, oldEntryPath];
}

type EntryPath = string;
type OldEntryPath = string;
export type FsWatcherEvent = [FsWatcherEventType, EntryPath, OldEntryPath?];

export interface FsWatcherOptions {
  path: string,
  ignoredPaths: string[],

  onEvent?(event: FsWatcherEvent): void,
  onEvents?(events: FsWatcherEvent[]): void,

  logger?: (...args: any[]) => void,
}

export class FsWatcher implements IDisposableObject {
  private watcherId: number = -1;
  private ignoredPaths: string[];
  private logger: (...args: any[]) => void;

  protected onEvent = (event: FsWatcherEvent) => this.logger('Event!', event);
  protected onEvents?: (events: FsWatcherEvent[]) => void;

  constructor(options: FsWatcherOptions) {
    this.ignoredPaths = options.ignoredPaths;
    this.logger = options.logger || ((...args) => console.log(...args));

    if (options.onEvent) {
      this.onEvent = options.onEvent;
    }

    if (options.onEvents) {
      this.onEvents = options.onEvents;
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
    const eventForPath: Record<string, FsWatcherEvent | null> = Object.create(null);

    for (const rawEvent of rawEvents) {
      const [action, fromEntryPath, toEntryPath] = normalizeRawEvent(rawEvent);

      // Untill we have "complete" git integration - don't care about that
      // TODO: Git integration
      if (fromEntryPath.indexOf('\\.git\\') > -1 || rawEvent[2] === '.git') {
        continue;
      }

      if (this.ignoredPaths.some((ignoredPath) => fromEntryPath.indexOf(ignoredPath) > -1)) {
        continue;
      }

      if (action === FsWatcherEventType.RENAMED) {
        eventForPath[fromEntryPath] = null;
        eventForPath[toEntryPath!] = [action, toEntryPath!, fromEntryPath];
        continue;
      }

      const event = eventForPath[fromEntryPath];

      // Event is obsolete
      if (event === null) {
        continue;
      }

      // No event yet - record it
      if (!event) {
        eventForPath[fromEntryPath] = [action, fromEntryPath];
        continue;
      }

      // CREATED -> DELETED in one batch - obsolete
      if (action === FsWatcherEventType.DELETED && event[0] === FsWatcherEventType.CREATED) {
        eventForPath[fromEntryPath] = null;
        continue;
      }

      // MODIFIED|RENAMED -> DELETED in one batch - only keep deleted
      if (action === FsWatcherEventType.DELETED && event[0] > FsWatcherEventType.DELETED) {
        eventForPath[fromEntryPath] = [action, fromEntryPath];
        continue;
      }
    }

    const events: FsWatcherEvent[] = Object.values(eventForPath)
      .filter(Boolean)
      .sort((a, b) => b![1].length - a![1].length) as any;

    if (!events.length) {
      return;
    }

    if (this.onEvents) {
      return this.onEvents(events);
    } else {
      for (const event of events) {
        this.onEvent(event);
      }
    }
  };

  private handleError = (error: string) => {
    this.logger('WATCHER ERROR!!! ', error);
  };
}
