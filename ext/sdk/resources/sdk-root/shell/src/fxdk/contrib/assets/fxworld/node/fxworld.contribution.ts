import { registerApiContribution } from "backend/api/api.extensions";
import { registerSingleton } from "backend/container-access";
import { IFsEntryHandler, ProjectFsExtensions } from "fxdk/project/node/projectExtensions";
import { IFsEntry } from "fxdk/project/common/project.types";
import { FXWORLD_ENTRY_HANDLE } from "../common/fxworld-constants";
import { FXWORLD_FILE_EXT } from "../common/fxworld-types";
import { FXWorldAssetRuntime } from "./fxworldAssetRuntime";
import { FXWorldCreatorService } from "./fxworldCreatorService";

registerApiContribution(
  registerSingleton(FXWorldCreatorService)
);

ProjectFsExtensions.register(FXWORLD_ENTRY_HANDLE, class FXWorldHandler implements IFsEntryHandler {
  handles(entry: IFsEntry) {
    return !entry.isDirectory && entry.name.endsWith(FXWORLD_FILE_EXT);
  }

  spawnAssetRuntime(fsEntry: IFsEntry, fsEntryPath: string) {
    return new FXWorldAssetRuntime(fsEntry, fsEntryPath);
  }
});
