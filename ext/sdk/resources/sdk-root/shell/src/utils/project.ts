import { FilesystemEntry } from "shared/api.types";
import { assetMetaFileExt } from "shared/asset.types";
import { ProjectData } from "shared/project.types";

export const filesystemEntryToTheiaResource = (entry: FilesystemEntry): string => {
  return 'file:///' + entry.path.replace('\\', '/');
};

export const isReadOnlyEntry = (entry: FilesystemEntry): boolean => {
  return !!entry.meta.assetMeta?.flags?.readOnly;
};

export const getFoldersForTheia = (project: ProjectData, pendingDeletions: Set<string>): string[] => {
  const entriesToInspect = new Set(project.fs[project.path]);
  const editableFolders: string[] = [];

  for (const entry of entriesToInspect) {
    if (!entry.isDirectory) {
      continue;
    }

    if (entry.name[0] === '.') {
      continue;
    }

    if (pendingDeletions.has(entry.path)) {
      continue;
    }

    editableFolders.push(filesystemEntryToTheiaResource(entry));
  }

  return editableFolders;
};

export const stripAssetMetaExt = (str: string): string => {
  return str.substr(0, str.indexOf(assetMetaFileExt));
};

export const isAssetMetaFile = (str: string): boolean => {
  const index = str.indexOf(assetMetaFileExt);
  if (index === -1) {
    return false;
  }

  return (index + assetMetaFileExt.length) === str.length;
};

export function isParentAssetPath(entryPath: string, assetPath: string): boolean {
  // entry path doesn't start with asset path - skip
  if (entryPath.indexOf(assetPath) !== 0) {
    return false;
  }

  // Asset can't be a parent if it's path is longer
  if (assetPath.length > entryPath.length) {
    return false;
  }

  const nextChar = entryPath.charAt(assetPath.length);

  // if entry's next char after asset's last char isn't path separator - not a parent asset
  if (nextChar !== '/' && nextChar !== '\\') {
    return false;
  }

  return true;
}

export function isChildAssetPath(entryPath: string, assetPath: string): boolean {
  // entry asset doesn't start with entry path - skip
  if (assetPath.indexOf(entryPath) !== 0) {
    return false;
  }

  // Asset can't be a child if it's path is less than entry's
  if (assetPath.length < entryPath.length) {
    return false;
  }

  const nextChar = assetPath.charAt(entryPath.length);

  // if asset's next char after entry's last char isn't path separator - not a child asset
  if (nextChar !== '/' && nextChar !== '\\') {
    return false;
  }

  return true;
}
