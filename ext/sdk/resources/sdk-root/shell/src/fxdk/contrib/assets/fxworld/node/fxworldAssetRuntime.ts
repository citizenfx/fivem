import { AssetDeployablePathsDescriptor, AssetRuntime } from "fxdk/project/node/asset/asset-runtime";
import { ApiClientScoped } from "backend/api/api-client-scoped";
import { handlesClientEvent } from "backend/api/api-decorators";
import { lazyInject } from "backend/container-access";
import { ServerResourceDescriptor } from "backend/game-server/game-server-runtime";
import { WorldEditorMapCompiler } from "backend/world-editor/world-editor-map-compiler";
import { WorldEditorService } from "backend/world-editor/world-editor-service";
import { fxworldRecompile } from "../common/fxworld-constants";
import { FXWORLD_FILE_EXT } from "../common/fxworld-types";

export class FXWorldAssetRuntime extends AssetRuntime {
  @lazyInject(ApiClientScoped)
  protected readonly apiClient: ApiClientScoped;

  @lazyInject(WorldEditorService)
  protected readonly worldEditorService: WorldEditorService;

  @lazyInject(WorldEditorMapCompiler)
  protected readonly worldEditorMapCompiler: WorldEditorMapCompiler;

  protected postContstruct() {
    this.apiClient.init(this);
    this.apiClient.setScope(this.getName());
  }

  @handlesClientEvent(fxworldRecompile)
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
      mapFilePath: this.fsEntryPath,
      targetPath: this.getMapResourcePath(),
    });
  }
}
