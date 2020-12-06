import { AssetCreateRequest } from 'shared/api.requests';
import { ApiClient } from 'shared/api.types';
import { AssetApi } from '../../api/AssetApi';
import { ProjectApi } from '../../api/ProjectApi';

export interface IAssetKind {
  create(request: AssetCreateRequest): Promise<boolean>;
}

export interface IAssetManager extends IAssetKind {
}

export class BaseAssetKind implements IAssetKind {
  constructor(
    protected readonly client: ApiClient,
    protected readonly asset: AssetApi,
    protected readonly project: ProjectApi,
  ) { }

  create(request: AssetCreateRequest): Promise<boolean> {
    throw new Error('Must be implemented!');
  }
}

export class BaseAssetManager implements IAssetManager {
  constructor(
    protected readonly client: ApiClient,
    protected readonly asset: AssetApi,
    protected readonly project: ProjectApi,
  ) { }

  create(request: AssetCreateRequest): Promise<boolean> {
    throw new Error('Must be implemented!');
  }
}
