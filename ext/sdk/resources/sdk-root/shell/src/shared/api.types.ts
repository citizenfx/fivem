import { AssetMeta } from "./asset.types";

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

export type FilesystemEntryMap = {
  [path: string]: FilesystemEntry[],
};


export enum Feature {
  windowsDevModeEnabled,
  dotnetAvailable,
}

export type FeaturesMap = Partial<Record<Feature, boolean>>;
