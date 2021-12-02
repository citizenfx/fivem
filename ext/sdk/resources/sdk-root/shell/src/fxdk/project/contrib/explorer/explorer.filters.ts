import { IFsEntry } from "fxdk/project/common/project.types";
import { isAssetMetaFile } from "utils/project";

export function fsEntriesFilter(entry: IFsEntry): boolean {
  if (isAssetMetaFile(entry.name)) {
    return false;
  }

  return true;
}

export function rootFsEntriesFilter(entry: IFsEntry): boolean {
  if (entry.name === '.fxserver') {
    return false;
  }

  if (entry.name === '.fxdk') {
    return false;
  }

  if (entry.name === '.vscode') {
    return false;
  }

  if (entry.name === 'fxproject.json') {
    return false;
  }

  return fsEntriesFilter(entry);
}
