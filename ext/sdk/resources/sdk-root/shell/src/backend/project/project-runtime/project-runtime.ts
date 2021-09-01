import { ContainerAccess } from "backend/container-access";
import { disposableFromFunction, DisposableObject } from "backend/disposable-container";
import { FsWatcherEventType } from "backend/fs/fs-watcher";
import { Task } from "backend/task/task-reporter-service";
import { inject, injectable } from "inversify";
import { FilesystemEntry, FilesystemEntryMap } from "shared/api.types";
import { ProjectAssetBaseConfig, ProjectManifest } from "shared/project.types";
import { concurrently } from "utils/concurrently";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { ProjectAssetsRuntime } from "./project-assets-runtime";
import { ProjectFsRuntime } from "./project-fs-runtime";
import { ProjectManifestRuntime } from "./project-manifest-runtime";
import { ProjectPathsStateRuntime } from "./project-paths-state-runtime";
import { ProjectStateRuntime } from "./project-state-runtime";

export type AssetConfigChangedListener = <T extends ProjectAssetBaseConfig>(config: T) => void;
export interface AssetConfigChangedEvent {
  assetPath: string,
  config: ProjectAssetBaseConfig,
}

@injectable()
export class ProjectRuntime {
  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  public state: ProjectStateRuntime;
  public assets: ProjectAssetsRuntime;
  public pathsState: ProjectPathsStateRuntime;
  public fs: ProjectFsRuntime;

  private manifest: ProjectManifestRuntime;

  initState(projectPath: string) {
    this.state = this.containerAccess.resolve(ProjectStateRuntime);
    this.state.init(projectPath);
  }

  async init(task: Task) {
    this.pathsState = this.containerAccess.resolve(ProjectPathsStateRuntime);
    this.manifest = this.containerAccess.resolve(ProjectManifestRuntime);
    this.assets = this.containerAccess.resolve(ProjectAssetsRuntime);
    this.fs = this.containerAccess.resolve(ProjectFsRuntime);

    await this.pathsState.init(this);
    await this.manifest.init(this);
    await this.assets.init(this);
    await this.fs.init(this);
  }

  async dispose() {
    return concurrently(
      this.fs.dispose(),
      this.assets.dispose(),
      this.manifest.dispose(),
      this.pathsState.dispose(),
    );
  }

  public getFSMap(): FilesystemEntryMap {
    return this.fs.get();
  }

  public getManifest(): ProjectManifest {
    return this.manifest.get();
  }

  public applyManifest(fn: (manifest: ProjectManifest) => void) {
    this.manifest.apply(fn);
  }

  private assetConfigChangedEvent = new SingleEventEmitter<AssetConfigChangedEvent>();
  private assetConfigChangeListeners: Record<string, AssetConfigChangedListener> = {};
  public onAnyAssetConfigChanged(cb: (event: AssetConfigChangedEvent) => void) {
    this.assetConfigChangedEvent.addListener(cb);
  }
  public onAssetConfigChanged(assetPath: string, cb: AssetConfigChangedListener): DisposableObject {
    this.assetConfigChangeListeners[assetPath] = cb;

    return disposableFromFunction(() => delete this.assetConfigChangeListeners[assetPath]);
  }
  public emitAssetConfigChanged(assetPath: string, config: ProjectAssetBaseConfig) {
    this.assetConfigChangeListeners[assetPath]?.(config);
    this.assetConfigChangedEvent.emit({ assetPath, config });
  }

  public readonly manifestUpdated = new SingleEventEmitter<ProjectManifest>();

  private fsEntryMetaContributions: Record<string, (entryPath: string) => Promise<any>> = {};
  public registerEntryMetaContribution(meta: string, contrib: (entryPath: string) => Promise<any>) {
    this.fsEntryMetaContributions[meta] = contrib;
  }
  public getEntryMetaContributions() {
    return this.fsEntryMetaContributions;
  }

  public forceFSScan(path: string) {
    this.fs.forceScan(path);
  }

  public readonly fsEntry = new SingleEventEmitter<FilesystemEntry>();
  public readonly fsUpdate = new SingleEventEmitter<{ type: FsWatcherEventType, path: string, entry: FilesystemEntry | null }>();
  public readonly fsEntryDeleted = new SingleEventEmitter<string>();
  public readonly fsEntryRenamed = new SingleEventEmitter<{ entry: FilesystemEntry, oldPath: string | void }>();
  public readonly fsEntryMoved = new SingleEventEmitter<{ newPath: string, oldPath: string }>();
}
