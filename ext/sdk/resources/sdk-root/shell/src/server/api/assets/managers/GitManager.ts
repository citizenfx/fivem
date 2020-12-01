import * as path from 'path';
import simpleGitPromised from 'simple-git/promise';
import { invariant } from '../../../invariant';
import { assetKinds, assetManagerTypes, assetStatus } from 'shared/api.types';
import { BaseAssetManager } from "../types";
import { AssetCreateRequest } from 'shared/api.requests';

export class GitManager extends BaseAssetManager {
  async create(request: AssetCreateRequest): Promise<boolean> {
    if (!this.project.projectInstance) {
      return false;
    }

    const {
      assetName,
      managerData,
      readOnly,
    } = request;

    invariant(managerData?.repoUrl, `Can't create git managed asset without managerData.repoUrl`);

    this.client.log('Creating asset managed by git', request);

    const assetPath = path.join(request.assetPath, assetName);
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
    await this.project.projectInstance.setAssetMeta(assetPath, assetMeta, { forceShadow: true });

    const git = simpleGitPromised(request.assetPath);

    try {
      this.client.log('Importing git asset', request);

      await git.clone(managerData?.repoUrl, assetName);
      this.client.log('Done: Importing git asset', request);

      if (request.callback) {
        request.callback();
      }

      assetMeta.manager.data.status = assetStatus.ready;
      await this.project.projectInstance.setAssetMeta(assetPath, assetMeta, { forceShadow: true });

      this.project.projectInstance.readAndNotifyFsTree();

      return true;
    } catch (e) {
      assetMeta.manager.data.status = assetStatus.error;
      await this.project.projectInstance.setAssetMeta(assetPath, assetMeta, { forceShadow: true });

      this.client.log('Importing git asset ERROR', e, request);

      return false;
    }
  }
}
