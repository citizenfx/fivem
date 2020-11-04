import { FilesystemEntry } from "sdkApi/api.types";


export const projectExplorerItemType = {
  FILE: 'projectExplorer:file',
  FOLDER: 'projectExplorer:folder',
};

export interface EntryMoveItem {
  entry?: FilesystemEntry,
  type: string,
}
