import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { bindAsset } from "./asset/asset-bindings";
import { ProjectManager } from "./project-manager";
import { ProjectAccess } from "./project-access";
import { ProjectBuilder } from "./project-builder";
import { ProjectUpgrade } from "./project-upgrade";
import { ProjectAssetManagers } from "./project-asset-managers";
import { ProjectAssets } from "./project-assets";

export const bindProject = (container: interfaces.Container) => {
  // Always create new instance
  container.bind(ProjectAssets).toDynamicValue((ctx) => ctx.container.resolve(ProjectAssets));

  container.bind(ProjectAccess).toSelf().inSingletonScope();
  container.bind(ProjectUpgrade).toSelf().inSingletonScope();
  container.bind(ProjectAssetManagers).toSelf().inSingletonScope();

  container.bind(ProjectManager).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectManager);

  container.bind(ProjectBuilder).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectBuilder);

  bindAsset(container);
};
