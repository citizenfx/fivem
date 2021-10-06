import { FilesystemEntry } from "shared/api.types";


export const projectExplorerItemType = {
  FILE: 'projectExplorer:file',
  FOLDER: 'projectExplorer:folder',
  ASSET: 'projectExplorer:asset',
};

export interface NativeDropItem {
  files: (File & { path: string })[],
}

export interface FxDKDropItem {
  entry?: FilesystemEntry,
  type: string | Symbol,
}
export type EntryMoveItem =
  | NativeDropItem
  | FxDKDropItem;
