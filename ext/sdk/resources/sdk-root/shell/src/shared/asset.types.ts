export interface AssetMetaFlags {
  readOnly?: boolean,
}

export interface AssetMeta {
  flags?: AssetMetaFlags,
  data?: object,
}

export const assetMetaFileExt = '.fxmeta';
