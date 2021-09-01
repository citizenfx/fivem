import { inject, injectable } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { LogService } from 'backend/logger/log-service';
import { FsService } from 'backend/fs/fs-service';
import { invariant } from 'utils/invariant';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { NotificationService } from 'backend/notification/notification-service';
import { ProjectAccess } from 'backend/project/project-access';
import { AssetImporterContribution } from '../../asset-importer-contribution';
import { ReleaseAssetImportRequest } from './release-importer.types';
import { GameServerInstallerUtils } from 'backend/game-server/game-server-installer-utils';
import filesize from 'filesize';
import { join } from 'path';

@injectable()
export class ReleaseImporter implements AssetImporterContribution {
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

  @inject(GameServerInstallerUtils)
  protected readonly gameServerInstallerUtils: GameServerInstallerUtils;

  async importAsset(request: ReleaseAssetImportRequest): Promise<boolean> {
    const {
      assetName,
      assetMetaFlags,
      data,
    } = request;

    invariant(data?.releaseUrl, `Invalid release asset import request, missing releaseUrl in data`);

    const importTask = this.taskReporterService.create('Importing asset from git');
    importTask.setText('Preparing');

    const { releaseUrl } = data;

    this.logService.log('Importing asset from release', request);

    if (assetMetaFlags) {
      const assetPath = this.fsService.joinPath(request.assetBasePath, assetName);

      await this.projectAccess.withInstance(async (project) => {
        await project.setAssetMeta(assetPath, {
          flags: assetMetaFlags,
        });
      });
    }

    const assetPath = join(request.assetBasePath, assetName);
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

            importTask.setText(`Downloaded: ${filesize(doneDownloadSize)}`);
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

              importTask.setText(`Unpacked: ${filesize(doneUnpackSize)}`);
              importTask.setProgress(doneUnpackSize / totalUnpackSize);
            },
          );
        } catch (e) {
          this.notificationService.error(`Failed to import release: ${e.toString()}`);
          return false;
        }
      }

      if (request.callback) {
        request.callback();
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
