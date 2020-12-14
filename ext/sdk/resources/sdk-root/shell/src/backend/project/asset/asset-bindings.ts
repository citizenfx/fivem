import { interfaces } from "inversify";
import { bindContributionProvider } from "backend/contribution-provider";
import { AssetContribution, bindAssetContribution } from "./asset-contribution";
import { ResourceManager } from "./contributions/resource-manager";
import { GitManager } from "./contributions/git-manager";

export const bindAsset = (container: interfaces.Container) => {
  bindContributionProvider(container, AssetContribution);

  container.bind(ResourceManager).toSelf().inSingletonScope();
  bindAssetContribution(container, ResourceManager, 'resource');

  container.bind(GitManager).toSelf().inSingletonScope();
  bindAssetContribution(container, GitManager, 'git');
};
