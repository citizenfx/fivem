import { Api } from "fxdk/browser/Api";
import { ProjectApi } from "fxdk/project/common/project.api";
import { APIRQ } from "shared/api.requests";
import { EntryMoveItem } from "./explorer.dnd";

export function copyItemIntoEntryFromDrop(entryPath: string, item: EntryMoveItem) {
  const item_ANY: any = item;

  // Thanks ts for not allowing narrowing non-discriminative unions...
  // Handling native files drop from desktop
  if (Array.isArray(item_ANY?.files)) {
    const targetPath = entryPath;
    const sourcePaths: string[] = item_ANY.files.map((file) => file.path).filter((sourcePath) => sourcePath !== targetPath);

    if (sourcePaths.length && targetPath) {
      const request: APIRQ.CopyEntries = {
        sourcePaths,
        targetPath,
      };

      return Api.send(ProjectApi.FsEndpoints.copyEntries, request);
    }

    return;
  }

  const sourcePath = item_ANY?.entryPath;
  const targetPath = entryPath;

  if (sourcePath && targetPath && sourcePath !== targetPath) {
    const moveEntryRequest: APIRQ.MoveEntry = {
      sourcePath,
      targetPath,
    };

    Api.send(ProjectApi.FsEndpoints.moveEntry, moveEntryRequest);
  }
}

export function renameFileSystemEntry(entryPath: string, newName: string) {
  const request: APIRQ.RenameEntry = {
    entryPath,
    newName,
  };

  Api.send(ProjectApi.FsEndpoints.renameEntry, request);
}
