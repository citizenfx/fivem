import { ProjectManifestResource, ServerUpdateChannel } from "./api.types";


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

export type AssetCreateAction = 'create' | 'import';
export interface AssetCreateRequest<T = any> {
  action: AssetCreateAction,
  managerName: string,
  assetPath: string,
  assetName: string,
  readOnly?: boolean,
  data?: T,
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
  licenseKey?: string,
  steamWebApiKey?: string,
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

export interface ProjectBuildRequest {
  buildPath: string,
  useVersioning: boolean,
}
