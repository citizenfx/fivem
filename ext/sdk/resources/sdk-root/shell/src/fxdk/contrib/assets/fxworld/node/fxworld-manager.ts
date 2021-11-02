import { IFsEntryHandler, ProjectFsExtensions } from "fxdk/project/node/projectExtensions";
import { IFsEntry } from "fxdk/project/common/project.types";
import { FXWorldAssetRuntime } from "./fxworldAssetRuntime";
import { FXWORLD_ENTRY_HANDLE } from "../common/fxworld-constants";
import { FXWORLD_FILE_EXT } from "../common/fxworld-types";

class FXWorldHandler implements IFsEntryHandler {
  handles(entry: IFsEntry) {
    return !entry.isDirectory && entry.name.endsWith(FXWORLD_FILE_EXT);
  }

  spawnAssetRuntime(fsEntry: IFsEntry, fsEntryPath: string) {
    return new FXWorldAssetRuntime(fsEntry, fsEntryPath);
  }
}

ProjectFsExtensions.register(FXWORLD_ENTRY_HANDLE, FXWorldHandler);
