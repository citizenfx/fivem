import { interfaces } from "inversify";
import { bindContributionProvider } from "backend/contribution-provider";
import { AssetManagerContribution, bindAssetContribution } from "./asset-manager-contribution";
import { AssetImporterContribution, bindAssetImporterContribution } from "./asset-importer-contribution";
import { assetImporterTypes, assetTypes } from "shared/asset.types";
import { ResourceManager } from "./asset-contributions/resource/resource-manager";
import { GitImporter } from "./importer-contributions/git-importer/git-importer";
import { FsImporter } from "./importer-contributions/fs-importer/fs-importer";

export const bindAsset = (container: interfaces.Container) => {
  bindContributionProvider(container, AssetManagerContribution);
  bindContributionProvider(container, AssetImporterContribution);

  container.bind(ResourceManager).toSelf().inSingletonScope();
  bindAssetContribution(container, ResourceManager, assetTypes.resource);

  container.bind(GitImporter).toSelf().inSingletonScope();
  bindAssetImporterContribution(container, GitImporter, assetImporterTypes.git);

  container.bind(FsImporter).toSelf().inSingletonScope();
  bindAssetImporterContribution(container, FsImporter, assetImporterTypes.fs);
};
