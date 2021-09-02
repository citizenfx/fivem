import { fxdkProjectFilename } from "backend/constants";
import { FsService } from "backend/fs/fs-service";
import { inject, injectable } from "inversify";
import { PROJECT_PATHS_STATE_FILENAME } from "../project-constants";

export enum ProjectState {
  Development,
  Building,
}

@injectable()
export class ProjectStateRuntime {
  @inject(FsService)
  protected readonly fsService: FsService;

  private state: ProjectState = ProjectState.Development;

  private _path: string;
  get path(): string {
    return this._path;
  }

  private _pathsStatePath: string;
  get pathsStatePath(): string {
    return this._pathsStatePath;
  }

  private _manifestPath: string;
  get manifestPath(): string {
    return this._manifestPath;
  }

  private _storagePath: string;
  get storagePath(): string {
    return this._storagePath;
  }

  private _fxserverCwd: string;
  get fxserverCwd(): string {
    return this._fxserverCwd;
  }

  init(path: string) {
    this._path = path;

    this._manifestPath = this.fsService.joinPath(this._path, fxdkProjectFilename);

    this._storagePath = this.fsService.joinPath(this._path, '.fxdk');
    this._pathsStatePath = this.fsService.joinPath(this._storagePath, PROJECT_PATHS_STATE_FILENAME);

    this._fxserverCwd = this.fsService.joinPath(this._storagePath, 'fxserver');
  }

  getState(): ProjectState {
    return this.state;
  }

  setState(state: ProjectState) {
    this.state = state;
  }

  getPathInProject(nestedPath: string): string {
    return this.fsService.joinPath(this._path, nestedPath);
  }

  getPathInStorage(nestedPath: string): string {
    return this.fsService.joinPath(this._storagePath, nestedPath);
  }
}
