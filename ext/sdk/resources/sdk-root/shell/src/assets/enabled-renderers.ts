import { assetTypes } from "shared/asset.types";
import { FXWorld } from "./fxworld/renderer/FXWorld";
import { Resource } from "./resource/renderer/Resource";

export const ENABLED_ASSET_RENDERERS = {
  [assetTypes.resource]: Resource,
  [assetTypes.fxworld]: FXWorld,
};
