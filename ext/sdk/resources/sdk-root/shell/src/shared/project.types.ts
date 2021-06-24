import { SystemResource } from "backend/system-resources/system-resources-constants";
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
  assetDefs: {
    [path: string]: any,
  },
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
  systemResources: SystemResource[],
  variables?: {
    [key: string]: string
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
}

export type ProjectBuildError =
  | { type: 'generic', data: string }
  | { type: 'assetBuildError', data: { assetName: string, outputChannelId: string } };
