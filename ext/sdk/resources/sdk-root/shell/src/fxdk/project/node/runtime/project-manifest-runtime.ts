import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { FsJsonFileMapping, FsJsonFileMappingOptions } from "backend/fs/fs-json-file-mapping";
import { FsService } from "backend/fs/fs-service";
import { serverUpdateChannels } from "shared/api.types";
import { ProjectAssetBaseConfig, ProjectManifest } from "shared/project.types";
import { ProjectRuntime } from "./project-runtime";
import { ProjectEvents, ProjectFsEvents } from "../project-events";
import { lazyInject } from "backend/container-access";
import { ApiClient } from "backend/api/api-client";
import { ProjectApi } from "fxdk/project/common/project.api";
import { ScopedLogService } from "backend/logger/scoped-logger";

export class ProjectManifestRuntime implements IDisposableObject {
  @lazyInject(FsService)
  protected readonly fsService: FsService;

  @lazyInject(ApiClient)
  protected readonly apiClient: ApiClient;

  protected readonly logService = new ScopedLogService('ProjectManifestRuntime');

  protected rt: ProjectRuntime;

  private manifestMapping: FsJsonFileMapping<ProjectManifest>;

  private toDispose = new Disposer();

  async init(rt: ProjectRuntime) {
    this.rt = rt;

    const options: FsJsonFileMappingOptions<ProjectManifest> = {
      defaults: {
        assets: {},
        serverUpdateChannel: serverUpdateChannels.recommended,
        systemResources: [],
      },
      onApply: (manifest) => {
        this.apiClient.emit(ProjectApi.ManifestEndpoints.update, this.get());
        ProjectEvents.ManifestUpdated.emit(manifest);
      },
    };

    this.manifestMapping = await this.fsService.createJsonFileMapping<ProjectManifest>(this.rt.state.manifestPath, options);

    // Handling asset renames
    this.toDispose.register(ProjectFsEvents.BeforeRename.addListener(async (event) => {
      const oldAssetRelativePath = this.getRelativeAssetPath(event.oldEntryPath);
      const newAssetRelativePath = this.getRelativeAssetPath(event.entryPath);

      if (!this.get().assets[oldAssetRelativePath]) {
        this.logService.log(`FS entry is about to rename, but we do not have it's asset config - skipping manifest update`);
        return;
      }

      await this.apply((manifest) => {
        manifest.assets[newAssetRelativePath] = manifest.assets[oldAssetRelativePath];

        delete manifest.assets[oldAssetRelativePath];
      });
    }));
  }

  dispose() {
    dispose(this.toDispose);
  }

  apply(fn: (manifest: ProjectManifest) => void) {
    return this.manifestMapping.apply(fn);
  }

  get(): ProjectManifest {
    return this.manifestMapping.get();
  }

  getAssetConfig<T extends ProjectAssetBaseConfig>(assetPath: string, defaultConfig: T): T {
    return {
      ...defaultConfig,
      ...this.get().assets[this.getRelativeAssetPath(assetPath)],
    };
  }

  setAssetConfig<T extends ProjectAssetBaseConfig>(assetPath: string, config: T) {
    this.apply((manifest) => {
      manifest.assets[this.getRelativeAssetPath(assetPath)] = config;
    });

    ProjectEvents.AssetConfigChanged.emit({ assetPath, config });
  }

  private getRelativeAssetPath(assetPath: string): string {
    return assetPath.substr(this.rt.state.path.length + FsService.separator.length);
  }
}
