import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { bindAsset } from "./asset/asset-bindings";
import { ProjectManager } from "./project-manager";
import { ProjectAccess } from "./project-access";
import { ProjectBuilder } from "./project-builder";

export const bindProject = (container: interfaces.Container) => {
  container.bind(ProjectAccess).toSelf().inSingletonScope();

  container.bind(ProjectManager).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectManager);

  container.bind(ProjectBuilder).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectBuilder);

  bindAsset(container);
};
