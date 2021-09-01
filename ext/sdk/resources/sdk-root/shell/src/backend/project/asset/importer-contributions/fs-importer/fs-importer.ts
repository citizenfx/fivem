import { ApiClient } from 'backend/api/api-client';
import { CopyOptions, FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { NotificationService } from 'backend/notification/notification-service';
import { ProjectAccess } from 'backend/project/project-access';
import { Task, TaskReporterService } from 'backend/task/task-reporter-service';
import { injectable, inject } from 'inversify';
import { AssetImporterContribution } from '../../asset-importer-contribution';
import { FsAssetImportRequest } from './fs-importer.types';

@injectable()
export class FsImporter implements AssetImporterContribution {
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

  async importAsset(request: FsAssetImportRequest): Promise<boolean> {
    const importTask = this.taskReporterService.create(`Importing asset`);

    try {
      await this.doImport(importTask, request);
      this.notificationService.info(`Succefully imported asset from ${request.data?.sourcePath}`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Failed to import asset: ${e.toString()}`);
      this.logService.log('Failed to import asset', e, request);
      return false;
    } finally {
      importTask.done();
    }
  }

  private async doImport(task: Task,request: FsAssetImportRequest) {
    const assetPath = this.fsService.joinPath(request.assetBasePath, request.assetName);

    const sourcePath = request.data?.sourcePath;
    if (!sourcePath) {
      throw new Error(`Source path for import is not specified`);
    }

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
