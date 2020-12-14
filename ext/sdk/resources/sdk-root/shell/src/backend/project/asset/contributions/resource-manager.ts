import { inject, injectable, interfaces } from 'inversify';
import { AssetCreateRequest } from 'shared/api.requests';
import { AssetContribution, AssetCreator, AssetInterface } from '../asset-contribution';
import { Project } from 'backend/project/project';
import { ApiClient } from 'backend/api/api-client';
import { LogService } from 'backend/logger/log-service';
import { FsService } from 'backend/fs/fs-service';
import { FilesystemEntry } from 'shared/api.types';
import { ContainerAccess } from 'backend/container-access';
import { resourceManifestFilename, resourceManifestLegacyFilename } from 'backend/constants';
import { fastRandomId } from 'utils/random';
import { GameServerService } from 'backend/game-server/game-server-service';
import { FsUpdateType } from 'backend/fs/fs-mapping';


interface IdealResourceMetaData {
  client_script: string[];
  client_scripts: string[];
  shared_script: string[];
  shared_scripts: string[];
  server_script: string[];
  server_scripts: string[];
};
export type ResourceMetaData = Partial<IdealResourceMetaData>;

const restartInducingFields = ['client_script', 'client_scripts', 'shared_script', 'shared_scripts', 'server_script', 'server_scripts'];

const AssetEntry = Symbol('AssetEntry');

@injectable()
export class ResourceManager implements AssetContribution, AssetCreator {
  readonly name: 'resource';
  readonly capabilities = {
    create: true,
  };

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  async createAsset(project: Project,request: AssetCreateRequest): Promise<boolean> {
    this.logService.log('Creating resource asset', request);

    const resourcePath = this.fsService.joinPath(request.assetPath, request.assetName);

    await this.fsService.mkdirp(resourcePath);

    const fxmanifestPath = this.fsService.joinPath(resourcePath, 'fxmanifest.lua');
    const fxmanifestContent = `
fx_version 'cerulean'
games { 'gta5' }
`.trimStart();

    project.applyManifest((manifest) => {
      manifest.resources[request.assetName] = {
        name: request.assetName,
        enabled: true,
        restartOnChange: false,
      };
    });

    await this.fsService.writeFile(fxmanifestPath, fxmanifestContent)

    this.logService.log('Finish creating asset', request);

    if (request.callback) {
      request.callback();
    }

    return true;
  }

  async loadAsset(project: Project, assetEntry: FilesystemEntry): Promise<AssetInterface | void> {
    if (!assetEntry.meta.isResource) {
      return;
    }

    return this.containerAccess.withChildContainer((container: interfaces.Container) => {
      container.bind(Project).toConstantValue(project);
      container.bind(AssetEntry).toConstantValue(assetEntry);

      return container.resolve(Resource);
    });
  }
}

@injectable()
export class Resource implements AssetInterface {
  get name(): string {
    return this.entry.name;
  }

  get path(): string {
    return this.entry.path;
  }

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  protected manifestPath: string;

  protected metaData: ResourceMetaData = {};

  protected restartInducingPaths: string[] = [];

  constructor(
    @inject(Project)
    protected readonly project: Project,

    @inject(AssetEntry)
    protected entry: FilesystemEntry,

    @inject(LogService)
    protected readonly logService: LogService,

    @inject(FsService)
    protected readonly fsService: FsService,
  ) {
    this.init();
  }

  setEntry(assetEntry: FilesystemEntry) {
    this.entry = assetEntry;
  }

  async onFsEvent(event: FsUpdateType, entry: FilesystemEntry | null) {
    const isChange = event === FsUpdateType.change;

    const { enabled, restartOnChange } = this.project.getResourceConfig(this.entry.name);
    if (!enabled) {
      this.logService.log('Resource is disabled', this.name);
      return;
    }

    if (isChange && entry?.path === this.manifestPath) {
      this.logService.log('Reloading meta data as manifest changed', this.name);
      this.loadMetaData();
      return this.gameServerService.refreshResources();
    }

    this.logService.log('Restart inducing paths', this.name, this.restartInducingPaths);

    if (isChange && restartOnChange && this.restartInducingPaths.includes(entry?.path)) {
      this.logService.log(`[Resource ${this.name}] Restarting resource`, this.name);
      this.gameServerService.restartResource(this.name);
    }
  }

  private async init() {
    const manifestPath = this.fsService.joinPath(this.path, resourceManifestFilename);
    const legacyManifestPath = this.fsService.joinPath(this.path, resourceManifestLegacyFilename);

    if (await this.fsService.statSafe(manifestPath)) {
      this.manifestPath = manifestPath;
    } else if (await this.fsService.statSafe(legacyManifestPath)) {
      this.manifestPath = legacyManifestPath;
    }

    this.loadMetaData();
  }

  private async loadMetaData(): Promise<void> {
    this.logService.log('Loading resource meta data', this.name);

    return new Promise((resolve, reject) => {
      const uid = this.name + '-' + fastRandomId();
      const timeout = setTimeout(() => {
        reject(`Resource ${this.path} meta data load timed out after 5 seconds`);
      }, 5000);

      const cb = (resName: string, metaData: ResourceMetaData) => {
        if (resName === uid) {
          clearTimeout(timeout);
          RemoveEventHandler('sdk:resourceMetaDataResponse', cb);

          this.logService.log('Loaded meta data for asset', uid, metaData);

          this.metaData = metaData;
          this.processMetaData();

          return resolve();
        }
      };

      on('sdk:resourceMetaDataResponse', cb);
      emit('sdk:requestResourceMetaData', this.path, uid);
    });
  }

  private processMetaData() {
    try {
      this.restartInducingPaths = restartInducingFields
        .map((field) => this.metaData[field] || [])
        .flat()
        .map((relativePath) => this.fsService.joinPath(this.path, relativePath));

      this.logService.log('Processed meta data', this.restartInducingPaths);
    } catch (e) {
      this.logService.log('Error while processing meta data', e);
    }
  }
}
