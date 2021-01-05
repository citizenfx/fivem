import simpleGitPromised from 'simple-git/promise';
import { assetKinds, assetManagerTypes, assetStatus, FilesystemEntry } from 'shared/api.types';
import { AssetCreateRequest } from 'shared/api.requests';
import { AssetContribution, AssetInterface } from '../asset-contribution';
import { inject, injectable } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { LogService } from 'backend/logger/log-service';
import { FsService } from 'backend/fs/fs-service';
import { Project } from 'backend/project/project';
import { invariant } from 'utils/invariant';
import { TaskReporterService } from 'backend/task/task-reporter-service';
import { NotificationService } from 'backend/notification/notification-service';
import { ProjectAccess } from 'backend/project/project-access';

@injectable()
export class GitManager implements AssetContribution {
  readonly name = 'git';
  readonly capabilities = {
    import: true,
  };

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

  async importAsset(request: AssetCreateRequest<{ repoUrl: string }>): Promise<boolean> {
    const {
      assetName,
      data,
      readOnly,
    } = request;

    invariant(data.repoUrl, `Can't create git managed asset without managerData.repoUrl`);

    const importTask = this.taskReporterService.create('Importing git asset');
    importTask.setText('Preparing')

    const { repoUrl } = data;

    this.logService.log('Creating asset managed by git', request);

    const assetPath = this.fsService.joinPath(request.assetPath, assetName);
    const assetMeta = {
      kind: assetKinds.pack,
      manager: {
        type: assetManagerTypes.git,
        data,
      },
      flags: {
        readOnly,
      },
    };

    importTask.setText('Creating meta data');
    // Asset meta for imported git pack asset is always in shadow root
    await this.projectAccess.getInstance().setAssetMeta(assetPath, assetMeta, { forceShadow: true });

    const git = simpleGitPromised(request.assetPath);

    try {
      this.logService.log('Importing git asset', request);

      importTask.setText(`Cloning repository ${repoUrl}`);

      await git.clone(repoUrl, assetName);
      this.logService.log('Done: Importing git asset', request);

      if (request.callback) {
        request.callback();
      }

      importTask.setText('Done');

      return true;
    } catch (e) {
      this.notificationService.error(`Git asset import failed: ${e.toString()}`);
      this.logService.log('Importing git asset ERROR', e, request);

      return false;
    } finally {
      importTask.done();
    }
  }

  loadAsset(assetEntry: FilesystemEntry): AssetInterface | void {
    return;
  }
}
