import { FilesystemEntry, FilesystemEntryMap, ServerUpdateChannel } from "./api.types";


export interface ProjectResource extends ProjectManifestResource {
  path: string,
  running: boolean,
}

export type ProjectResources = {
  [path: string]: ProjectResource,
};

export interface ProjectManifestResource {
  name: string,
  enabled: boolean,
  restartOnChange: boolean,
}

export type ProjectPathsState = {
  [path: string]: boolean,
};

export interface ProjectData {
  path: string,
  manifest: ProjectManifest,

  fs: FilesystemEntryMap,
  resources: ProjectResources,
}

export interface ProjectManifest {
  name: string,
  createdAt: string,
  updatedAt: string,
  serverUpdateChannel: ServerUpdateChannel,
  resources: {
    [name: string]: ProjectManifestResource,
  },
  pathsState: ProjectPathsState,
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
