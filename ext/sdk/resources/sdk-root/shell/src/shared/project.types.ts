import { AssetName, IFsEntry, IProjectVariable, ProjectPathsState } from "fxdk/project/common/project.types";
import { SystemResource } from "backend/system-resources/system-resources-constants";
import { ServerUpdateChannel } from "./api.types";

export interface ProjectOpenData {
  path: string,
  manifest: ProjectManifest,
  pathsState: ProjectPathsState,
  rootFsEntry: IFsEntry,
}

export interface ProjectAssetBaseConfig {
  enabled: boolean,
}

export interface ProjectManifest {
  name: string,
  createdAt: string,
  serverUpdateChannel: ServerUpdateChannel,
  assets: {
    [path: string]: ProjectAssetBaseConfig,
  },
  systemResources: SystemResource[],
  variables?: Record<AssetName, IProjectVariable>,
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
