import { inject, injectable } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { LogService } from 'backend/logger/log-service';
import { FsService } from 'backend/fs/fs-service';
import { invariant } from 'utils/invariant';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { NotificationService } from 'backend/notification/notification-service';
import { ProjectAccess } from 'backend/project/project-access';
import { AssetImporterContribution } from '../../asset-importer-contribution';
import { GitAssetImportRequest } from './git-importer.types';
import { GitService } from 'backend/git/git-service';

@injectable()
export class GitImporter implements AssetImporterContribution {
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

  @inject(GitService)
  protected readonly gitService: GitService;

  async importAsset(request: GitAssetImportRequest): Promise<boolean> {
    const {
      assetName,
      assetMetaFlags,
      data,
    } = request;

    invariant(data.repoUrl, `Invalid git asset import request, missing repoUrl in data`);

    const importTask = this.taskReporterService.create('Importing asset from git');
    importTask.setText('Preparing');

    const { repoUrl } = data;

    this.logService.log('Importing asset from git', request);

    if (assetMetaFlags) {
      const assetPath = this.fsService.joinPath(request.assetBasePath, assetName);

      await this.projectAccess.withInstance(async (project) => {
        await project.setAssetMeta(assetPath, {
          flags: assetMetaFlags,
        });
      });
    }

    try {
      this.logService.log('Importing git asset', request);

      importTask.setText(`Cloning repository ${repoUrl}`);

      await this.gitService.clone(this.fsService.joinPath(request.assetBasePath, assetName), repoUrl);

      this.logService.log('Done: Importing git asset', request);

      if (request.callback) {
        request.callback();
      }

      importTask.setText('Done');

      this.notificationService.info(`Succefully imported asset from ${repoUrl}`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Git asset import failed: ${e.toString()}`);
      this.logService.log('Importing git asset ERROR', e, request);

      return false;
    } finally {
      importTask.done();
    }
  }
}
