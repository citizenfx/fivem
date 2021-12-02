import { ApiClient } from "backend/api/api-client";
import { lazyInject } from "backend/container-access";
import { FsService } from "backend/fs/fs-service";
import { ServerResourceDescriptor } from "backend/game-server/game-server-runtime";
import { GameServerService } from "backend/game-server/game-server-service";
import { GameService } from "backend/game/game-service";
import { ScopedLogService } from "backend/logger/scoped-logger";
import { NotificationService } from "backend/notification/notification-service";
import { OutputService } from "backend/output/output-service";
import { ProjectInstanceService } from "fxdk/project/node/project";
import { ProjectAccess } from "fxdk/project/node/project-access";
import { ProjectFsEvents } from "fxdk/project/node/project-events";
import { IFsEntry } from "fxdk/project/common/project.types";
import { StatusService } from "backend/status/status-service";
import { dispose, Disposer, IDisposable, IDisposableObject } from "fxdk/base/disposable";
import { ProjectAssetBaseConfig } from "shared/project.types";
import { ProjectApi } from "fxdk/project/common/project.api";

export class AssetBuildCommandError {
  constructor(
    public readonly assetName: string,
    public readonly outputChannelId: string,
  ) { }
}


export interface AssetDeployablePathsDescriptor {
  root: string,
  paths: string[],
}

export interface IAssetRuntime<ConfigType extends ProjectAssetBaseConfig = ProjectAssetBaseConfig> extends IDisposableObject {
  readonly handle: string;
  readonly fsEntry: IFsEntry;
  readonly fsEntryPath: string;

  init(): Promise<void> | void;
  registerDisposable<T extends IDisposable>(disposable: T): T;

  acceptNestedFsEntrySpawned?(event: ProjectFsEvents.FsEntrySpawnedEvent): void;
  acceptNestedFsEntryUpdated?(event: ProjectFsEvents.FsEntryUpdatedEvent): void;
  acceptNestedFsEntryChildrenScanned?(event: ProjectFsEvents.FsEntryChildrenScannedEvent): void;

  acceptNestedAfterCreated?(event: ProjectFsEvents.AfterCreatedEvent): void;
  acceptNestedAfterDeleted?(event: ProjectFsEvents.AfterDeletedEvent): void;
  acceptNestedAfterModified?(event: ProjectFsEvents.AfterModifiedEvent): void;
  acceptNestedBeforeRename?(event: ProjectFsEvents.BeforeRenamedEvent): void;
  acceptNestedAfterRenamed?(event: ProjectFsEvents.AfterRenamedEvent): void;

  getName(): string;

  handleConfigChanged?(config: ConfigType): Promise<void> | void;

  getDeployablePathsDescriptor?(): Promise<AssetDeployablePathsDescriptor>;

  getResourceDescriptor?(): ServerResourceDescriptor | void;

  build?(): Promise<void> | void;
}

export abstract class AssetRuntime<ConfigType extends ProjectAssetBaseConfig = ProjectAssetBaseConfig> implements IAssetRuntime<ConfigType> {
  public readonly handle: string;

  public readonly defaultConfig: ProjectAssetBaseConfig = {
    enabled: true,
  };

  @lazyInject(GameService)
  protected readonly gameService: GameService;

  @lazyInject(GameServerService)
  protected readonly gameServerService: GameServerService;

  @lazyInject(FsService)
  protected readonly fsService: FsService;

  @lazyInject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @lazyInject(NotificationService)
  protected readonly notificationService: NotificationService;

  @lazyInject(StatusService)
  protected readonly statusService: StatusService;

  @lazyInject(OutputService)
  protected readonly outputService: OutputService;

  protected readonly logService: ScopedLogService;

  protected readonly toDispose = new Disposer();
  registerDisposable<T extends IDisposable>(disposable: T): T {
    return this.toDispose.register(disposable);
  }

  protected readonly project: ProjectInstanceService;

  constructor(
    public readonly fsEntry: IFsEntry,
    public readonly fsEntryPath: string,
  ) {
    this.handle = fsEntry.handle;
    this.project = this.projectAccess.getInstance();
    this.logService = new ScopedLogService(`asset ${this.getName()}`);

    this.postContstruct();
  }

  dispose() {
    return dispose(this.toDispose);
  }

  getName(): string {
    return this.fsEntry.name;
  }

  protected getDefaultConfig?(): ConfigType;

  init() {
    // noop by default
  }

  protected postContstruct() {
    // noop by default
  }

  protected getConfig(): ConfigType {
    return {
      ...(this.getDefaultConfig?.() || null),
      ...this.project.getAssetConfig(this.fsEntryPath),
    };
  }

  protected setConfig(config: Partial<ConfigType>) {
    this.project.setAssetConfig(this.fsEntryPath, config);
  }
}

export class AssetRuntimeDataSource<T> implements IDisposableObject {
  @lazyInject(ApiClient)
  protected readonly apiClient: ApiClient;

  constructor(
    private assetPath: string,
    private data: T,
  ) {
    this.apiClient.emit(ProjectApi.AssetEndpoints.setAssetRuntimeData, {
      assetPath: this.assetPath,
      data: this.data,
    });
  }

  dispose() {
    this.apiClient.emit(ProjectApi.AssetEndpoints.deleteAssetRuntimeData, { assetPath: this.assetPath });
  }

  apply(fn: (data: T) => void) {
    fn(this.data);

    this.apiClient.emit(ProjectApi.AssetEndpoints.setAssetRuntimeData, {
      assetPath: this.assetPath,
      data: this.data,
    });
  }
}
