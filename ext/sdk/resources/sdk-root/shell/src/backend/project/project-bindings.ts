import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { bindAsset } from "./asset/asset-bindings";
import { ProjectManager } from "./project-manager";
import { ProjectAccess } from "./project-access";
import { ProjectBuilder } from "./project-builder";
import { ProjectUpgrade } from "./project-upgrade";
import { ProjectResourcesMaintainer } from "./project-resources-maintainer";

export const bindProject = (container: interfaces.Container) => {
  container.bind(ProjectAccess).toSelf().inSingletonScope();
  container.bind(ProjectUpgrade).toSelf().inSingletonScope();
  container.bind(ProjectResourcesMaintainer).toSelf().inSingletonScope();

  container.bind(ProjectManager).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectManager);

  container.bind(ProjectBuilder).toSelf().inSingletonScope();
  bindApiContribution(container, ProjectBuilder);

  bindAsset(container);
};
