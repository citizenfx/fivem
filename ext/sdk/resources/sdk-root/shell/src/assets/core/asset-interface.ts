import { DisposableObject } from "backend/disposable-container";
import { FsWatcherEventType } from "backend/fs/fs-watcher";
import { ServerResourceDescriptor } from "backend/game-server/game-server-runtime";
import { FilesystemEntry } from "shared/api.types";
import { AssetType } from "shared/asset.types";

export interface AssetDeployablePathsDescriptor {
  root: string,
  paths: string[],
}

export interface AssetInterface extends DisposableObject {
  readonly type: AssetType;

  getName(): string;
  getPath(): string;

  setEntry?(entry: FilesystemEntry): Promise<void> | void;
  onFsUpdate?(updateType: FsWatcherEventType, entry: FilesystemEntry | null, entryPath: string): Promise<void> | void;

  getDeployablePathsDescriptor?(): Promise<AssetDeployablePathsDescriptor>;

  getResourceDescriptor?(): ServerResourceDescriptor | void;
  getDefinition?(): Record<string, unknown>;

  build?(): Promise<void> | void;
}
