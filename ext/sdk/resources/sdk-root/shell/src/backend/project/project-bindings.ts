import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { bindAsset } from "./asset/asset-bindings";
import { ProjectManager } from "./project-manager";

export const bindProject = (container: interfaces.Container) => {
  container.bind(ProjectManager).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectManager);

  bindAsset(container);
};
