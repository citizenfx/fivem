import { AssetImporterType, AssetMetaFlags, AssetType } from "./asset.types";
import { ProjectAssetBaseConfig } from "./project.types";


export interface ProjectCreateRequest {
  projectName: string,
  projectPath: string,
}

export interface ProjectSetAssetConfigRequest<AssetConfigType extends ProjectAssetBaseConfig = ProjectAssetBaseConfig> {
  assetPath: string,
  config: Partial<AssetConfigType>,
}

export interface ProjectCreateDirectoryRequest {
  directoryPath: string,
  directoryName: string,
}

export interface DeleteDirectoryRequest {
  directoryPath: string,
  hardDelete?: boolean,
}
export enum DeleteDirectoryResponse {
  Ok,
  FailedToRecycle,
}

export interface ProjectRenameDirectoryRequest {
  directoryPath: string,
  newDirectoryName: string,
}

export interface ProjectCreateFileRequest {
  filePath: string,
  fileName: string,
}

export interface DeleteFileRequest {
  filePath: string,
  hardDelete?: boolean,
}
export enum DeleteFileResponse {
  Ok,
  FailedToRecycle,
}

export interface ProjectRenameFileRequest {
  filePath: string,
  newFileName: string,
}

export interface AssetCreateRequest<T extends object = object> {
  assetType: AssetType,
  assetPath: string,
  assetName: string,
  assetMetaFlags?: AssetMetaFlags,
  data?: T,
  callback?: () => void,
}

export interface AssetImportRequest<T extends object = object> {
  importerType: AssetImporterType,
  assetName: string,
  assetBasePath: string,
  assetMetaFlags?: AssetMetaFlags,
  data?: T,
  callback?: () => void,
}

export interface FetchReleasesRequest {
  repoUrl: string,
}

export interface ReleaseInfo {
  name: string,
  body: string,
  createdAt: string,
  downloadUrl: string,
}

export interface FetchReleasesResponse {
  success: boolean,
  releases: ReleaseInfo[],
}

export interface AssetDeleteRequest {
  assetPath: string,
  hardDelete: boolean,
}
export enum AssetDeleteResponse {
  Ok,
  FailedToRecycle,
}

export interface AssetRenameRequest {
  assetPath: string,
  newAssetName: string,
}

export interface RelinkResourcesRequest {
  projectPath: string,
  enabledResourcesPaths: string[],
}

export interface ProjectStartServerRequest {
  licenseKey?: string,
  steamWebApiKey?: string,
  tebexSecret?: string,
}

export interface MoveEntryRequest {
  sourcePath: string,
  targetPath: string,
}

export interface CopyEntriesRequest {
  sourcePaths: string[];
  targetPath: string
}

export type CopyEntryRequest = MoveEntryRequest;

export interface ProjectBuildRequest {
  buildPath: string,
  useVersioning: boolean,
  deployArtifact: boolean,
  steamWebApiKey: string,
  tebexSecret: string,
}
