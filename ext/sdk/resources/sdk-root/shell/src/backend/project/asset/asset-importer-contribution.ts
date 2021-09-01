import { interfaces } from 'inversify';
import { APIRQ } from 'shared/api.requests';
import { AssetImporterType } from 'shared/asset.types';

export const AssetImporterContribution = Symbol('AssetImporterContribution');
export interface AssetImporterContribution {
  importAsset(request: APIRQ.AssetImport): Promise<boolean>;
}

export const bindAssetImporterContribution = <T>(container: interfaces.Container, service: interfaces.Newable<T>, importerType: AssetImporterType) => {
  container.bind(AssetImporterContribution).to(service).inSingletonScope().whenTargetTagged('importerType', importerType);
};
