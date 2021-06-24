import { DisposableObject } from "backend/disposable-container";
import { FsWatcherEventType } from "backend/fs/fs-watcher";
import { ServerResourceDescriptor } from "backend/game-server/game-server-runtime";
import { FilesystemEntry } from "shared/api.types";
import { AssetType } from "shared/asset.types";

export interface AssetInterface extends DisposableObject {
  readonly type: AssetType;

  getName(): string;
  getPath(): string;

  setEntry?(entry: FilesystemEntry): Promise<void> | void;
  onFsUpdate?(updateType: FsWatcherEventType, entry: FilesystemEntry | null, entryPath: string): Promise<void> | void;

  getDeployablePaths?(): Promise<string[]>;

  getResourceDescriptor?(): ServerResourceDescriptor;
  getDefinition?(): any;

  build?(): Promise<void> | void;
}
