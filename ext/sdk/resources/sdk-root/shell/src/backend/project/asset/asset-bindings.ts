import { interfaces } from "inversify";
import { bindContributionProvider } from "backend/contribution-provider";
import { AssetContribution, bindAssetContribution } from "./asset-contribution";
import { ResourceManager } from "./resource/resource-manager";
import { GitManager } from "./contributions/git-manager";
import { bindResource } from "./resource/resource-bindings";

export const bindAsset = (container: interfaces.Container) => {
  bindContributionProvider(container, AssetContribution);

  bindResource(container);

  container.bind(GitManager).toSelf().inSingletonScope();
  bindAssetContribution(container, GitManager, 'git');
};
