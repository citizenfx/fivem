import { ContainerAccess } from "backend/container-access";
import { AssetManagerContribution } from "backend/project/asset/asset-manager-contribution";
import { inject, injectable } from "inversify";
import { FilesystemEntry } from "shared/api.types";
import { endsWith } from "utils/stringUtils";
import { FXWorld } from "./fxworld";
import { FXWORLD_FILE_EXT } from "./fxworld-types";

@injectable()
export class FXWorldManager implements AssetManagerContribution {
  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  createAsset() {
    return null;
  }

  loadAsset(assetEntry: FilesystemEntry): FXWorld | void {
    if (!assetEntry.isFile || !endsWith(assetEntry.path, FXWORLD_FILE_EXT)) {
      return;
    }

    const asset = this.containerAccess.resolve(FXWorld);

    asset.setEntry(assetEntry);

    return asset;
  }
}
