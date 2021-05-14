import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { SystemResourcesService } from "./system-resources-service";

export const bindSystemResources = (container: interfaces.Container) => {
  container.bind(SystemResourcesService).toSelf().inSingletonScope();
  bindAppContribution(container, SystemResourcesService);
};
