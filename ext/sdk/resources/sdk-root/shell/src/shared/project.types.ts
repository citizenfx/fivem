import { FilesystemEntry, FilesystemEntryMap, ServerUpdateChannel } from "./api.types";
import { AssetType } from "./asset.types";


export type ProjectPathsState = {
  [path: string]: boolean,
};

export interface ProjectData {
  fs: FilesystemEntryMap,
  path: string,
  assets: {
    [path: string]: ProjectAssetBaseConfig,
  },
  assetTypes: Record<string, AssetType | void>,
  manifest: ProjectManifest,
}

export interface ProjectAssetBaseConfig {
  enabled: boolean,
}

export interface ProjectManifest {
  name: string,
  createdAt: string,
  updatedAt: string,
  serverUpdateChannel: ServerUpdateChannel,
  pathsState: ProjectPathsState,
  assets: {
    [path: string]: ProjectAssetBaseConfig,
  },
}

export interface ProjectFsUpdate {
  replace: {
    [path: string]: FilesystemEntry[],
  },
  delete: string[],
}

export interface RecentProject {
  name: string,
  path: string,
}

export interface ProjectCreateCheckResult {
  openProject?: boolean,
  ignoreCfxServerData?: boolean,
}

export type ProjectBuildError =
  | { type: 'generic', data: string }
  | { type: 'assetBuildError', data: { assetName: string, outputChannelId: string } };
