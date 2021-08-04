import { AssetDeployablePathsDescriptor, AssetInterface } from "assets/core/asset-interface";
import { ApiClientScoped } from "backend/api/api-client-scoped";
import { ApiContribution } from "backend/api/api-contribution";
import { FsService } from "backend/fs/fs-service";
import { ServerResourceDescriptor } from "backend/game-server/game-server-runtime";
import { ProjectAccess } from "backend/project/project-access";
import { WorldEditorMapCompiler } from "backend/world-editor/world-editor-map-compiler";
import { WorldEditorService } from "backend/world-editor/world-editor-service";
import { inject, injectable, postConstruct } from "inversify";
import { FilesystemEntry } from "shared/api.types";
import { assetTypes } from "shared/asset.types";
import { FXWORLD_FILE_EXT } from "./fxworld-types";

@injectable()
export class FXWorld implements AssetInterface, ApiContribution {
  public readonly type = assetTypes.fxworld;

  getId() {
    return this.getName();
  }

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @inject(ApiClientScoped)
  protected readonly apiClient: ApiClientScoped;

  @inject(WorldEditorService)
  protected readonly worldEditorService: WorldEditorService;

  @inject(WorldEditorMapCompiler)
  protected readonly worldEditorMapCompiler: WorldEditorMapCompiler;

  @postConstruct()
  initApiClient() {
    this.apiClient.init(this);
  }

  private entry: FilesystemEntry;

  getName() {
    return this.entry.name;
  }

  getPath() {
    return this.entry.path;
  }

  setEntry(entry: FilesystemEntry) {
    this.entry = entry;

    this.apiClient.setScope(this.entry.name);
  }

  async build() {
    return this.compileMap();
  }

  getResourceDescriptor(): ServerResourceDescriptor | void {
    return {
      name: this.getMapResourceName(),
      path: this.getMapResourcePath(),
    };
  }

  async getDeployablePathsDescriptor(): Promise<AssetDeployablePathsDescriptor> {
    return {
      root: this.getMapResourcePath(),
      paths: ['fxmanifest.lua', 'map.js'],
    };
  }

  private getMapResourcePath(): string {
    const project = this.projectAccess.getInstance();

    return this.fsService.joinPath(
      project.getStoragePath(),
      'map-resources',
      this.getMapResourceName(),
    );
  }

  private getMapResourceName(): string {
    const project = this.projectAccess.getInstance();

    return `${this.getName().replace(FXWORLD_FILE_EXT, '')}_${project.getName()}_fxworld`;
  }

  private async compileMap() {
    return this.worldEditorMapCompiler.compile({
      mapFilePath: this.getPath(),
      targetPath: this.getMapResourcePath(),
    });
  }
}
