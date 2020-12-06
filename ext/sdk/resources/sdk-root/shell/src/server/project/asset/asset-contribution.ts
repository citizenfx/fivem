import { interfaces } from "inversify";
import { AssetCreateRequest } from "shared/api.requests";
import { Project } from "../project";

interface AssetContributionInterface {
  create(project: Project, request: AssetCreateRequest): Promise<boolean>,
}

export const AssetManager = Symbol('AssetManager');
export interface AssetManager extends AssetContributionInterface {
}

export const AssetKind = Symbol('AssetKind');
export interface AssetKind extends AssetContributionInterface {
}

export const bindAssetKind = <T>(container: interfaces.Container, service: interfaces.Newable<T>, kindName: string) => {
  container.bind(AssetKind).to(service).inSingletonScope().whenTargetTagged('kindName', kindName);
};

export const bindAssetManager = <T>(container: interfaces.Container, service: interfaces.Newable<T>, managerName: string) => {
  container.bind(AssetManager).to(service).inSingletonScope().whenTargetTagged('managerName', managerName);
};
