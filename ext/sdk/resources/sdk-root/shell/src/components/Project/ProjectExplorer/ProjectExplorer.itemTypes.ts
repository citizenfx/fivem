import { FilesystemEntry } from "shared/api.types";


export const projectExplorerItemType = {
  FILE: 'projectExplorer:file',
  FOLDER: 'projectExplorer:folder',
};

export interface EntryMoveItem {
  entry?: FilesystemEntry,
  type: string,
}
