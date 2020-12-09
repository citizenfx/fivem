import simpleGitPromised from 'simple-git/promise';
import { assetKinds, assetManagerTypes, assetStatus } from 'shared/api.types';
import { AssetCreateRequest } from 'shared/api.requests';
import { AssetManager } from '../asset-contribution';
import { inject, injectable } from 'inversify';
import { ApiClient } from 'backend/api/api-client';
import { LogService } from 'backend/logger/log-service';
import { FsService } from 'backend/fs/fs-service';
import { Project } from 'backend/project/project';
import { invariant } from 'utils/invariant';

@injectable()
export class GitManager implements AssetManager {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  async create(project: Project, request: AssetCreateRequest): Promise<boolean> {
    const {
      assetName,
      managerData,
      readOnly,
    } = request;

    invariant(managerData?.repoUrl, `Can't create git managed asset without managerData.repoUrl`);

    this.logService.log('Creating asset managed by git', request);

    const assetPath = this.fsService.joinPath(request.assetPath, assetName);
    const assetMeta = {
      kind: assetKinds.pack,
      manager: {
        type: assetManagerTypes.git,
        data: {
          ...managerData,
          status: assetStatus.updating,
        },
      },
      flags: {
        readOnly,
      },
    };

    // Asset meta for imported git pack asset is always in shadow root
    await project.setAssetMeta(assetPath, assetMeta, { forceShadow: true });

    const git = simpleGitPromised(request.assetPath);

    try {
      this.logService.log('Importing git asset', request);

      await git.clone(managerData?.repoUrl, assetName);
      this.logService.log('Done: Importing git asset', request);

      if (request.callback) {
        request.callback();
      }

      assetMeta.manager.data.status = assetStatus.ready;
      await project.setAssetMeta(assetPath, assetMeta, { forceShadow: true });

      project.readAndNotifyFsTree();

      return true;
    } catch (e) {
      assetMeta.manager.data.status = assetStatus.error;
      await project.setAssetMeta(assetPath, assetMeta, { forceShadow: true });

      this.logService.log('Importing git asset ERROR', e, request);

      return false;
    }
  }
}
