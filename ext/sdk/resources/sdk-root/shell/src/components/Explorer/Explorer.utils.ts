import { closedDirectoryIcon, fileIcon, openDirectoryIcon, projectIcon, resourceIcon } from "constants/icons";
import { explorerApi } from "shared/api.events";
import { FilesystemEntry } from "shared/api.types";
import { sendApiMessage } from "utils/api";

export const getRelativePath = (basePath: string, path: string): string => {
  return path.replace(basePath, '').replace('\\', '').replace('/', '');
}

export const getEntryIcon = (entry: FilesystemEntry, open: boolean) => {
  if (entry.meta.isFxdkProject) {
    return projectIcon;
  }

  if (entry.meta.isResource) {
    return resourceIcon;
  }

  if (entry.isDirectory) {
    return open ? openDirectoryIcon : closedDirectoryIcon;
  } else {
    return fileIcon;
  }
}

export const readDrives = () => sendApiMessage(explorerApi.readRoots);

export const readDir = (dir: string, recursive = false) => {
  if (recursive) {
    return sendApiMessage(explorerApi.readDirRecursive, dir);
  }

  return sendApiMessage(explorerApi.readDir, dir);
}
