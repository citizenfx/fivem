import * as fs from 'fs';
import * as path from 'path';
import * as rimrafSync from 'rimraf';
import { promisify } from 'util';
import { assetApi } from '../../shared/api.events';
import { ProjectApi } from "./ProjectApi";
import { IAssetKind, IAssetManager } from './assets/types';
import { ResourceKind } from './assets/kinds/ResourceKind';
import { GitManager } from './assets/managers/GitManager';
import { invariant } from '../invariant';
import { SystemEvent, systemEvents } from './systemEvents';
import { ApiClient, AssetCreateRequest, AssetDeleteRequest, assetKinds, assetManagerTypes, AssetRenameRequest } from 'shared/api.types';

const rimraf = promisify(rimrafSync);

export class AssetApi {
  private kinds: {
    [kind: string]: IAssetKind,
  };

  private managers: {
    [type: string]: IAssetManager,
  };

  constructor(
    private readonly client: ApiClient,
    private readonly project: ProjectApi,
  ) {
    this.kinds = {
      [assetKinds.resource]: new ResourceKind(client, this, project),
    };
    this.managers = {
      [assetManagerTypes.git]: new GitManager(client, this, project),
    };

    this.client.on(assetApi.create, (request: AssetCreateRequest) => this.create(request));
    systemEvents.on(SystemEvent.assetCreateRequest, (request: AssetCreateRequest) => this.create(request));

    this.client.on(assetApi.rename, (request: AssetRenameRequest) => this.rename(request));
    systemEvents.on(SystemEvent.assetRenameRequest, (request: AssetRenameRequest) => this.rename(request));

    this.client.on(assetApi.delete, (request: AssetDeleteRequest) => this.delete(request));
    systemEvents.on(SystemEvent.assetDeleteRequest, (request: AssetDeleteRequest) => this.delete(request));
  }

  async create(request: AssetCreateRequest) {
    this.client.log('AssetApi:create()', request);

    if (request.managerType) {
      const managerImpl = this.managers[request.managerType];

      invariant(managerImpl, `No such asset manager: ${request.managerType}`);

      await managerImpl.create(request);
      return systemEvents.emit(SystemEvent.assetCreated, request);
    }

    if (request.assetKind) {
      const kindImpl = this.kinds[request.assetKind];

      invariant(kindImpl, `No such asset kind: ${request.assetKind}`);

      await kindImpl.create(request);
      return systemEvents.emit(SystemEvent.assetCreated, request);
    }

    this.client.log('Invalid asset create request, either assetKind or managerType must be specified');
    throw new Error('Invalid asset create request, either assetKind or managerType must be specified');
  }

  async rename(request: AssetRenameRequest) {
    if (!this.project.projectInstance) {
      return;
    }

    await this.project.projectInstance.handleAssetRename(request);

    const { assetPath, newAssetName } = request;

    const newAssetPath = path.join(path.dirname(assetPath), newAssetName);

    const promises = [
      fs.promises.rename(assetPath, newAssetPath),
    ];

    try {
      const oldShadowAssetPath = this.project.projectInstance.getPathInShadow(assetPath);
      const newShadowAssetPath = this.project.projectInstance.getPathInShadow(newAssetPath);

      await fs.promises.stat(oldShadowAssetPath);

      promises.push(
        fs.promises.rename(oldShadowAssetPath, newShadowAssetPath),
      );
    } catch (e) {

    }

    await Promise.all(promises);

    systemEvents.emit(SystemEvent.assetRenamed, request);
  }

  async delete(request: AssetDeleteRequest) {
    if (!this.project.projectInstance) {
      return;
    }

    await this.project.projectInstance.handleAssetDelete(request);

    const { assetPath } = request;

    await Promise.all([
      rimraf(assetPath),
      rimraf(this.project.projectInstance.getPathInShadow(assetPath)),
    ]);

    systemEvents.emit(SystemEvent.assetDeleted, request);
  }
}
