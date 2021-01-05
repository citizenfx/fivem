import { interfaces } from "inversify";
import { bindAssetContribution } from "../asset-contribution";
import { ResourceManager } from "./resource-manager";

export const bindResource = (container: interfaces.Container) => {
  container.bind(ResourceManager).toSelf().inSingletonScope();
  bindAssetContribution(container, ResourceManager, 'resource');
};
