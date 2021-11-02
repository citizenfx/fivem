import { ApiClient } from "backend/api/api-client";
import { Disposer, IDisposableObject } from "fxdk/base/disposable";
import { FsJsonFileMapping, FsJsonFileMappingOptions } from "backend/fs/fs-json-file-mapping";
import { FsService } from "backend/fs/fs-service";
import { ProjectRuntime } from "./project-runtime";
import { lazyInject } from "backend/container-access";
import { ProjectFsEvents } from "../project-events";
import { ProjectApi } from "fxdk/project/common/project.api";
import { ProjectPathsState } from "fxdk/project/common/project.types";

export class ProjectPathsStateRuntime implements IDisposableObject {
  @lazyInject(FsService)
  protected readonly fsService: FsService;

  @lazyInject(ApiClient)
  protected readonly apiClient: ApiClient;

  private rt: ProjectRuntime;

  private pathsStateMapping: FsJsonFileMapping<ProjectPathsState>;

  private disposed = false;
  private toDispose = new Disposer();

  private shouldEmitUpdate = false;

  async init(rt: ProjectRuntime) {
    this.rt = rt;

    this.toDispose.register(ProjectFsEvents.AfterRenamed.addListener((event) => {
      this.handleRenameAndMove(event.oldEntryPath, event.entryPath);
    }));

    const options: FsJsonFileMappingOptions<ProjectPathsState> = {
      defaults: {},
      onApply: this.maybeEmitPathsStateUpdate,
    };

    this.pathsStateMapping = await this.fsService.createJsonFileMapping<ProjectPathsState>(this.rt.state.pathsStatePath, options);

    this.sanitizePathsState();
  }

  async dispose() {
    this.disposed = true;
  }

  get(): ProjectPathsState {
    return this.pathsStateMapping.get();
  }

  apply(fn: (pathsState: ProjectPathsState) => void) {
    this.pathsStateMapping.apply(fn);
  }

  applyPartial(patch: ProjectPathsState) {
    this.apply((pathsState) => {
      for (const [key, value] of Object.entries(patch)) {
        pathsState[key] = value;
      }
    });
  }

  private async sanitizePathsState() {
    for (const path of Object.keys(this.get())) {
      if (this.disposed) {
        return;
      }

      if (!(await this.fsService.statSafe(path))) {
        if (this.disposed) {
          return;
        }

        this.apply((pathsState) => {
          delete pathsState[path];
        });
      }
    }
  }

  private handleRenameAndMove(oldPath: string, newPath: string) {
    this.apply((pathsState) => {
      for (const [path, state] of Object.entries(pathsState)) {
        if (path.indexOf(oldPath) === 0) {
          this.shouldEmitUpdate = true;

          delete pathsState[path];
          pathsState[path.replace(oldPath, newPath)] = state;
        }
      }
    });
  }

  private maybeEmitPathsStateUpdate = () => {
    if (!this.shouldEmitUpdate) {
      return;
    }

    this.shouldEmitUpdate = false;

    this.apiClient.emit(ProjectApi.FsEndpoints.pathsStateUpdate, this.pathsStateMapping.get());
  };
}
