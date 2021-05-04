import { assetTypes } from "shared/asset.types";
import { FXWorldManager } from "./fxworld/fxworld-manager";
import { ResourceManager } from "./resource/resource-manager";

export const ENABLED_ASSET_MANAGERS = {
  [assetTypes.resource]: ResourceManager,
  [assetTypes.fxworld]: FXWorldManager,
};
