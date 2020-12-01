import { AssetKind, AssetManagerType, ServerUpdateChannel } from "./api.types";


export interface ProjectCreateRequest {
  projectName: string,
  projectPath: string,
  withServerData?: boolean,
}

export interface AssetCreateRequest {
  assetPath: string,
  assetName: string,
  assetKind?: AssetKind,
  managerType?: AssetManagerType,
  managerData?: any,
  readOnly?: boolean,
  data?: any,
  callback?: Function,
}

export interface AssetDeleteRequest {
  assetPath: string,
}

export interface AssetRenameRequest {
  assetPath: string,
  newAssetName: string,
}

export interface RelinkResourcesRequest {
  projectPath: string,
  enabledResourcesPaths: string[],
}

export interface ServerStartRequest {
  projectPath: string,
  updateChannel: ServerUpdateChannel,
  enabledResourcesPaths: string[],
}

export interface ServerRefreshResourcesRequest {
  projectPath: string,
  enabledResourcesPaths: string[],
}

export interface MoveEntryRequest {
  sourcePath: string,
  targetPath: string,
}

export type CopyEntryRequest = MoveEntryRequest;
