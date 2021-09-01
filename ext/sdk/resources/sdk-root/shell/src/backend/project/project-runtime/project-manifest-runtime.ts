import { DisposableObject } from "backend/disposable-container";
import { FsJsonFileMapping, FsJsonFileMappingOptions } from "backend/fs/fs-json-file-mapping";
import { FsService } from "backend/fs/fs-service";
import { inject, injectable } from "inversify";
import { serverUpdateChannels } from "shared/api.types";
import { ProjectManifest } from "shared/project.types";
import { ProjectRuntime } from "./project-runtime";

@injectable()
export class ProjectManifestRuntime implements DisposableObject {
  @inject(FsService)
  protected readonly fsService: FsService;

  protected rt: ProjectRuntime;

  private manifestMapping: FsJsonFileMapping<ProjectManifest>;

  async init(rt: ProjectRuntime) {
    this.rt = rt;

    const options: FsJsonFileMappingOptions<ProjectManifest> = {
      defaults: {
        assets: {},
        serverUpdateChannel: serverUpdateChannels.recommended,
        systemResources: [],
      },
      onApply: (manifest) => this.rt.manifestUpdated.emit(manifest),
    };

    this.manifestMapping = await this.fsService.createJsonFileMapping<ProjectManifest>(this.rt.state.manifestPath, options);
  }

  async dispose() {

  }

  apply(fn: (manifest: ProjectManifest) => void) {
    this.manifestMapping.apply(fn);
  }


  get(): ProjectManifest {
    return this.manifestMapping.get();
  }
}
