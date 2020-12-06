import { AssetKind, AssetManagerType, ProjectManifestResource, ServerUpdateChannel } from "./api.types";


export interface ProjectCreateRequest {
  projectName: string,
  projectPath: string,
  withServerData?: boolean,
}

export interface ProjectSetResourceConfigRequest {
  resourceName: string,
  config: Partial<ProjectManifestResource>,
}

export interface ProjectCreateDirectoryRequest {
  directoryPath: string,
  directoryName: string,
}

export interface ProjectDeleteDirectoryRequest {
  directoryPath: string,
}

export interface ProjectRenameDirectoryRequest {
  directoryPath: string,
  newDirectoryName: string,
}

export interface ProjectCreateFileRequest {
  filePath: string,
  fileName: string,
}

export interface ProjectDeleteFileRequest {
  filePath: string,
}

export interface ProjectRenameFileRequest {
  filePath: string,
  newFileName: string,
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

export interface SetEnabledResourcesRequest {
  projectPath: string,
  enabledResourcesPaths: string[],
}

export interface MoveEntryRequest {
  sourcePath: string,
  targetPath: string,
}

export type CopyEntryRequest = MoveEntryRequest;
