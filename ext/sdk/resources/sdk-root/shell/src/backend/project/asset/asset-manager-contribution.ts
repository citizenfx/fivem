import { interfaces } from "inversify";
import { AssetCreateRequest } from "shared/api.requests";
import { FilesystemEntry } from "shared/api.types";
import { AssetType } from "shared/asset.types";
import { AssetInterface } from "assets/core/asset-interface";


export const AssetManagerContribution = Symbol('AssetManagerContribution');
export interface AssetManagerContribution {
  createAsset(request: AssetCreateRequest): Promise<boolean>;

  loadAsset(assetEntry: FilesystemEntry): AssetInterface | void;

  onFsEntry?(entry: FilesystemEntry): Promise<void> | void;
}

export const bindAssetContribution = <T>(container: interfaces.Container, service: interfaces.Newable<T>, assetType: AssetType) => {
  container.bind(AssetManagerContribution).to(service).inSingletonScope().whenTargetTagged('assetType', assetType);
};
