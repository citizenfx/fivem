import { FilesystemEntry, Project } from "../sdkApi/api.types";

export const filesystemEntryToTheiaResource = (entry: FilesystemEntry): string => {
  return 'file:///' + entry.path.replace('\\', '/');
};

export const isReadOnlyEntry = (entry: FilesystemEntry): boolean => {
  return !!entry.meta.assetMeta?.flags.readOnly;
};

export const getFoldersForTheia = (project: Project): string[] => {
  console.log('project entries', project.fsTree.entries);

  const entriesToInspect = new Set(project.fsTree.entries);
  const editableFolders: string[] = [];

  for (const entry of entriesToInspect) {
    if (!entry.isDirectory) {
      continue;
    }

    if (entry.name[0] === '.') {
      continue;
    }

    editableFolders.push(filesystemEntryToTheiaResource(entry));

    // if (entry.meta.isResource) {
    //   editableFolders.push(filesystemEntryToTheiaResource(entry));
    // } else {
    //   const entryChildren = project.fsTree.pathsMap[entry.path] || [];

    //   for (const entryChild of entryChildren) {
    //     entriesToInspect.add(entryChild);
    //   }
    // }
  }

  return editableFolders;
};
