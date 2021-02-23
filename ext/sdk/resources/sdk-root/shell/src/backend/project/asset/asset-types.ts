import { DisposableObject } from "backend/disposable-container";
import { FsUpdateType } from "backend/fs/fs-mapping";
import { FilesystemEntry } from "shared/api.types";

export interface AssetInterface extends DisposableObject {
  getId?(): string;

  setEntry?(entry: FilesystemEntry): Promise<void> | void;
  onFsUpdate?(updateType: FsUpdateType, entry: FilesystemEntry | null): Promise<void> | void;

  suspendWatchCommands?(): Promise<void> | void;
  resumeWatchCommands?(): Promise<void> | void;

  runBuildCommands?(): Promise<void> | void;
}
