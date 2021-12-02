import { ContainerAccess } from "backend/container-access";
import { dispose, Disposer, IDisposableObject } from "fxdk/base/disposable";
import { Task } from "backend/task/task-reporter-service";
import { inject, injectable } from "inversify";
import { ProjectAssetBaseConfig, ProjectManifest } from "shared/project.types";
import { ProjectManifestRuntime } from "./project-manifest-runtime";
import { ProjectPathsStateRuntime } from "./project-paths-state-runtime";
import { ProjectStateRuntime } from "./project-state-runtime";
import { ProjectFsController } from "./project-fs-controller";
import { ProjectAssetsController } from "./project-assets-controller";

export type AssetConfigChangedListener = <T extends ProjectAssetBaseConfig>(config: T) => void;
export interface AssetConfigChangedEvent {
  assetPath: string,
  config: ProjectAssetBaseConfig,
}

@injectable()
export class ProjectRuntime implements IDisposableObject {
  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  public fs: ProjectFsController;
  public assets: ProjectAssetsController;

  public state: ProjectStateRuntime;
  public pathsState: ProjectPathsStateRuntime;

  public manifest: ProjectManifestRuntime;

  private toDispose = new Disposer();

  initState(projectPath: string) {
    this.state = this.containerAccess.resolve(ProjectStateRuntime);
    return this.state.init(projectPath);
  }

  async init(task: Task) {
    this.pathsState = this.toDispose.register(new ProjectPathsStateRuntime());
    this.manifest = this.toDispose.register(new ProjectManifestRuntime());

    this.fs = this.toDispose.register(new ProjectFsController());
    this.assets = this.toDispose.register(new ProjectAssetsController());

    await this.pathsState.init(this);
    await this.manifest.init(this);

    await this.assets.init(this);
    await this.fs.init(this);
  }

  async dispose() {
    await dispose(this.toDispose);
  }

  public getManifest(): ProjectManifest {
    return this.manifest.get();
  }

  public applyManifest(fn: (manifest: ProjectManifest) => void) {
    this.manifest.apply(fn);
  }
}
