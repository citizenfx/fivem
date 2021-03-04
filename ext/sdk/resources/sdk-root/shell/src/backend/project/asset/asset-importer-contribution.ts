import { interfaces } from 'inversify';
import { AssetImportRequest } from 'shared/api.requests';
import { AssetImporterType } from 'shared/asset.types';

export const AssetImporterContribution = Symbol('AssetImporterContribution');
export interface AssetImporterContribution {
  importAsset(request: AssetImportRequest): Promise<boolean>;
}

export const bindAssetImporterContribution = <T>(container: interfaces.Container, service: interfaces.Newable<T>, importerType: AssetImporterType) => {
  container.bind(AssetImporterContribution).to(service).inSingletonScope().whenTargetTagged('importerType', importerType);
};
