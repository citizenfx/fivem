import { DisposableObject } from "backend/disposable-container";
import { FsWatcherEventType } from "backend/fs/fs-watcher";
import { FilesystemEntry } from "shared/api.types";

export interface AssetInterface extends DisposableObject {
  getId?(): string;

  getPath(): string;

  setEntry?(entry: FilesystemEntry): Promise<void> | void;
  onFsUpdate?(updateType: FsWatcherEventType, entry: FilesystemEntry | null): Promise<void> | void;

  suspendWatchCommands?(): Promise<void> | void;
  resumeWatchCommands?(): Promise<void> | void;

  runBuildCommands?(): Promise<void> | void;
}
