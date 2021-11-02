import { ApiClient } from "backend/api/api-client";
import { handlesClientEvent } from "backend/api/api-decorators";
import { CopyOptions, FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { NotificationService } from "backend/notification/notification-service";
import { ProjectAccess } from "fxdk/project/node/project-access";
import { Task, TaskReporterService } from "backend/task/task-reporter-service";
import { inject, injectable } from "inversify";
import { FsImporterApi } from "../common/importer.fs";

@injectable()
export class FsImporter {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @handlesClientEvent(FsImporterApi.Endpoints.import)
  async importAsset(request: FsImporterApi.ImportRequest): Promise<boolean> {
    const importTask = this.taskReporterService.create(`Importing asset`);

    try {
      await this.doImport(importTask, request);
      this.notificationService.info(`Succefully imported asset from ${request.sourcePath}`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Failed to import asset: ${e.toString()}`);
      this.logService.log('Failed to import asset', e, request);
      return false;
    } finally {
      importTask.done();
    }
  }

  private async doImport(task: Task, request: FsImporterApi.ImportRequest) {
    const { sourcePath } = request;
    const assetPath = this.fsService.joinPath(request.basePath, request.name);

    const sourcePathStats = await this.fsService.statSafe(sourcePath);
    if (!sourcePathStats) {
      throw new Error(`Source path ${sourcePath} to import does not exist`);
    }

    if (await this.fsService.statSafe(assetPath)) {
      throw new Error(`Can not import asset to existing path ${assetPath}`);
    }

    const copyOptions: CopyOptions = {
      onProgress(progress) {
        task.setProgress(progress);
      }
    };

    if (sourcePathStats.isDirectory()) {
      await this.fsService.mkdirp(assetPath);
      await this.fsService.copyDirContent(sourcePath, assetPath, copyOptions);
    } else {
      await this.fsService.copyFileContent(sourcePath, assetPath, copyOptions);
    }
  }
}
