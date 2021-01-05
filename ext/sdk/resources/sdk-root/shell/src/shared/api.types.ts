export enum AppStates {
  booting,
  preparing,
  ready,
}

export enum ServerStates {
  down,
  up,
  booting,
}

export enum ServerDataStates {
  checking,
  updating,
  ready,
}

export enum ServerUpdateStates {
  ready,
  checking,
  updateRequired,
  updating,
  missingArtifact,
}

export const serverUpdateChannels = {
  recommended: 'recommended',
  optional: 'optional',
  latest: 'latest',
};

export type ServerUpdateChannel = (typeof serverUpdateChannels)[keyof typeof serverUpdateChannels];

export interface ServerUpdateChannelsState {
  [updateChannel: string]: ServerUpdateStates,
}

export interface FilesystemEntryMeta {
  isFxdkProject?: boolean,
  isResource?: boolean,
  assetMeta?: AssetMeta | null,
}

export interface FilesystemEntry {
  path: string,
  name: string,
  meta: FilesystemEntryMeta,
  isFile: boolean,
  isDirectory: boolean,
  isSymbolicLink: boolean,
  children?: FilesystemEntry[],
}

export type ExplorerChildsMap = {
  [path: string]: FilesystemEntry[],
}

export const assetStatus = {
  ready: 'ready',
  updating: 'updating',
  error: 'error',
};

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

export type FilesystemEntryMap = {
  [path: string]: FilesystemEntry[],
};

export interface ProjectData {
  path: string,
  manifest: ProjectManifest,

  fs: FilesystemEntryMap,
  resources: ProjectResources,
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


export const assetManagerTypes = {
  none: 'none',
  git: 'git',
};
export type AssetManagerType = (typeof assetManagerTypes)[keyof typeof assetManagerTypes];
export interface AssetManager {
  type: AssetManagerType,
  data?: any,
}

export interface AssetMetaFlags {
  readOnly?: boolean,
}

interface BaseAssetMeta {
  flags: AssetMetaFlags,
  manager?: AssetManager,
}

export const assetKinds = {
  resource: 'resource',
  pack: 'pack',
};

export type AssetKind = (typeof assetKinds)[keyof typeof assetKinds];
export interface AssetMetaResource extends BaseAssetMeta {
  kind: typeof assetKinds.resource,
}
export interface AssetMetaPack extends BaseAssetMeta {
  kind: typeof assetKinds.pack,
}

export type AssetMeta =
  | AssetMetaResource
  | AssetMetaPack;


export enum Feature {
  systemGitClientAvailable,
  windowsDevModeEnabled,
  dotnetAvailable,
}

export type FeaturesMap = Partial<Record<Feature, boolean>>;

export interface ProjectCreateCheckResult {
  openProject?: boolean,
  ignoreCfxServerData?: boolean,
}
