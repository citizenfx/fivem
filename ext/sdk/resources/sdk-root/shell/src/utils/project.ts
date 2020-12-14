import { FilesystemEntry, ProjectData } from "shared/api.types";

export const filesystemEntryToTheiaResource = (entry: FilesystemEntry): string => {
  return 'file:///' + entry.path.replace('\\', '/');
};

export const isReadOnlyEntry = (entry: FilesystemEntry): boolean => {
  return !!entry.meta.assetMeta?.flags.readOnly;
};

export const getFoldersForTheia = (project: ProjectData): string[] => {
  const entriesToInspect = new Set(project.fs[project.path]);
  const editableFolders: string[] = [];

  for (const entry of entriesToInspect) {
    if (!entry.isDirectory) {
      continue;
    }

    if (entry.name[0] === '.') {
      continue;
    }

    editableFolders.push(filesystemEntryToTheiaResource(entry));
  }

  return editableFolders;
};
