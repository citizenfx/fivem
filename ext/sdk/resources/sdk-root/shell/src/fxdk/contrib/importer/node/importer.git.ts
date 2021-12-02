import { ApiClient } from "backend/api/api-client";
import { handlesClientEvent } from "backend/api/api-decorators";
import { FsService } from "backend/fs/fs-service";
import { GameServerInstallerUtils } from "backend/game-server/game-server-installer-utils";
import { GitService } from "backend/git/git-service";
import { ScopedLogService } from "backend/logger/scoped-logger";
import { NotificationService } from "backend/notification/notification-service";
import { ProjectAccess } from "fxdk/project/node/project-access";
import { TaskReporterService } from "backend/task/task-reporter-service";
import { inject, injectable } from "inversify";
import { GitImporterApi } from "../common/importer.git";

@injectable()
export class GitImporter {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  protected readonly logService = new ScopedLogService('Importer.Git');

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

  @inject(GameServerInstallerUtils)
  protected readonly gameServerInstallerUtils: GameServerInstallerUtils;

  @handlesClientEvent(GitImporterApi.Endpoints.importRepository)
  async importRepository(request: GitImporterApi.ImportRepositoryRequest): Promise<boolean> {
    const {
      basePath,
      name,
      repositoryUrl,
    } = request;

    const importTask = this.taskReporterService.create('Importing asset from git');
    importTask.setText('Preparing');

    this.logService.log('Importing asset from git', request);

    try {
      this.logService.log('Importing git asset', request);
      importTask.setText(`Cloning repository ${repositoryUrl}`);

      await this.gitService.clone(this.fsService.joinPath(basePath, name), repositoryUrl);

      this.logService.log('Done: Importing git asset', request);
      importTask.setText('Done');

      this.notificationService.info(`Succefully imported asset from ${repositoryUrl}`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Git asset import failed: ${e.toString()}`);
      this.logService.log('Importing git asset ERROR', e, request);

      return false;
    } finally {
      importTask.done();
    }
  }

  @handlesClientEvent(GitImporterApi.Endpoints.importRelease)
  async importAsset(request: GitImporterApi.ImportGithubReleaseRequest): Promise<boolean> {
    const {
      basePath,
      name,
      releaseUrl,
    } = request;

    const importTask = this.taskReporterService.create('Importing asset from git');
    importTask.setText('Preparing');

    this.logService.log('Importing asset from release', request);

    const assetPath = this.fsService.joinPath(basePath, name);
    const downloadingReleasePath = assetPath + '.zip';

    try {
      this.logService.log('Importing release asset', request);

      let totalDownloadSize = 0;
      let doneDownloadSize = 0;

      try {
        await this.gameServerInstallerUtils.downloadArtifact(
          releaseUrl,
          downloadingReleasePath,
          (totalSize) => totalDownloadSize = totalSize,
          (chunkSize) => {
            doneDownloadSize += chunkSize;

            importTask.setText(`Downloaded: ${this.fsService.filesizeToHumanReadable(doneDownloadSize)}`);
            importTask.setProgress(doneDownloadSize / totalDownloadSize);
          },
        );
      } catch (e) {
        this.notificationService.error(`Failed to import release: ${e.toString()}`);
        return false;
      }

      if (await this.fsService.statSafe(assetPath)) {
        await this.fsService.rimraf(assetPath);
      }

      {
        let totalUnpackSize = 0;
        let doneUnpackSize = 0;

        try {
          await this.gameServerInstallerUtils.unpackArtifact(
            downloadingReleasePath,
            assetPath,
            (totalSize) => totalUnpackSize = totalSize,
            (chunkSize) => {
              doneUnpackSize += chunkSize;

              importTask.setText(`Unpacked: ${this.fsService.filesizeToHumanReadable(doneUnpackSize)}`);
              importTask.setProgress(doneUnpackSize / totalUnpackSize);
            },
          );
        } catch (e) {
          this.notificationService.error(`Failed to import release: ${e.toString()}`);
          return false;
        }
      }

      importTask.setText('Done');

      this.notificationService.info(`Successfully imported asset from ${releaseUrl}`, 5000);
      return true;
    } catch (e) {
      this.notificationService.error(`Release asset import failed: ${e.toString()}`);
      this.logService.log('Importing release asset ERROR', e, request);

      return false;
    } finally {
      importTask.done();

      if (await this.fsService.statSafe(downloadingReleasePath)) {
        await this.fsService.unlink(downloadingReleasePath);
      }
    }
  }
}
