import { ProjectAssetBaseConfig, ProjectManifest } from "shared/project.types";
import { AsyncSingleEventEmitter } from "utils/singleEventEmitter";
import { ProjectRuntime } from "./runtime/project-runtime";
import { ProjectStateRuntime } from "./runtime/project-state-runtime";
import { IFsEntry } from "../common/project.types";
import { IAssetRuntime } from "./asset/asset-runtime";

export namespace ProjectEvents {
  export const Created = new AsyncSingleEventEmitter<ProjectStateRuntime>();
  export const Opened = new AsyncSingleEventEmitter<ProjectStateRuntime>();

  /**
   * Instead of listening on both Created and Opened one can listen on this event
   */
  export const BeforeLoad = new AsyncSingleEventEmitter<ProjectStateRuntime>();
  export const Loaded = new AsyncSingleEventEmitter<ProjectRuntime>();

  export const BeforeUnload = new AsyncSingleEventEmitter<ProjectRuntime>();

  export const ManifestUpdated = new AsyncSingleEventEmitter<ProjectManifest>();

  export const AssetSpawned = new AsyncSingleEventEmitter<IAssetRuntime>();

  export const AssetConfigChanged = new AsyncSingleEventEmitter<AssetConfigChangedEvent>();
  export type AssetConfigChangedEvent = { assetPath: string, config: ProjectAssetBaseConfig };
}

export namespace ProjectFsEvents {
  /**
   * Fires after initial project filesystem scan is done,
   * the only payload to listener is project's root IFsEntry
   */
  export const Hydrated = new AsyncSingleEventEmitter<IFsEntry>();

  /**
   * Fires when filesystem entry is discovered and converted to IFsEntry,
   * either during hydration or as a result of filesystem event
   */
  export const FsEntrySpawned = new AsyncSingleEventEmitter<FsEntrySpawnedEvent>();
  export type FsEntrySpawnedEvent = { fsEntry: IFsEntry, entryPath: string };

  /**
   * Fires when IFsEntry gets updated
   */
  export const FsEntryUpdated = new AsyncSingleEventEmitter<FsEntryUpdatedEvent>();
  export type FsEntryUpdatedEvent = { fsEntry: IFsEntry, entryPath: string };

  /**
   * Fires when IFsEntry children gets scanned
   */
  export const FsEntryChildrenScanned = new AsyncSingleEventEmitter<FsEntryChildrenScannedEvent>();
  export type FsEntryChildrenScannedEvent = { fsEntry: IFsEntry, entryPath: string };

  /**
   * Fires after filesystem entry gets created,
   * not to confuse with FsEntrySpawned as it will also be fired along with this event during entry creation
   */
  export const AfterCreated = new AsyncSingleEventEmitter<AfterCreatedEvent>();
  export type AfterCreatedEvent = { fsEntry: IFsEntry, entryPath: string };

  /**
   * Fires after filesystem entry gets changed: either meta information or content
   */
  export const AfterModified = new AsyncSingleEventEmitter<AfterModifiedEvent>();
  export type AfterModifiedEvent = { fsEntry: IFsEntry, entryPath: string };

  export const BeforeRename = new AsyncSingleEventEmitter<BeforeRenamedEvent>();
  export type BeforeRenamedEvent = { entryPath: string, oldEntryPath: string };

  /**
   * Fires after filesystem entry gets renamed,
   * this also means moved as it is essentially a rename,
   * won't be fired if something was moved outside of FxDK, though
   */
  export const AfterRenamed = new AsyncSingleEventEmitter<AfterRenamedEvent>();
  export type AfterRenamedEvent = { fsEntry: IFsEntry, entryPath: string, oldEntryPath: string };

  /**
   * Fires after filesystem entry gets deleted
   */
  export const AfterDeleted = new AsyncSingleEventEmitter<AfterDeletedEvent>();
  export type AfterDeletedEvent = { oldEntryPath: string };
}
