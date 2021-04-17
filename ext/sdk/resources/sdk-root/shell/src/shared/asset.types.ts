export interface AssetMetaFlags {
  readOnly?: boolean,
}

export type KeysOf<T> = T[keyof T];

export const assetTypes = {
  resource: 'resource',
  fxworld: 'fxworld',
};
export type AssetType = KeysOf<typeof assetTypes>;

export const assetImporterTypes = {
  git: 'git',
  release: 'release',
  fs: 'fs',
};
export type AssetImporterType = KeysOf<typeof assetImporterTypes>;

export interface AssetMeta {
  flags?: AssetMetaFlags,
  data?: object,
}

export const assetMetaFileExt = '.fxmeta';
