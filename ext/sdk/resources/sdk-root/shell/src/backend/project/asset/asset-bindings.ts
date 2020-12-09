import { interfaces } from "inversify";
import { bindContributionProvider } from "backend/contribution-provider";
import { AssetKind, AssetManager, bindAssetKind, bindAssetManager } from "./asset-contribution";
import { ResourceKind } from "./kinds/ResourceKind";
import { GitManager } from "./managers/GitManager";

export const bindAsset = (container: interfaces.Container) => {
  bindContributionProvider(container, AssetKind);
  bindContributionProvider(container, AssetManager);

  container.bind(ResourceKind).toSelf().inSingletonScope();
  bindAssetKind(container, ResourceKind, 'resource');

  container.bind(GitManager).toSelf().inSingletonScope();
  bindAssetManager(container, GitManager, 'git');
};
