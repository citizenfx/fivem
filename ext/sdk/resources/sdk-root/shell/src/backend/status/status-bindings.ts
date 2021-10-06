import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { StatusService } from "./status-service";
import { bindAppContribution } from "backend/app/app-contribution";

export const bindStatus = (container: interfaces.Container) => {
  container.bind(StatusService).toSelf().inSingletonScope();
  bindApiContribution(container, StatusService);
  bindAppContribution(container, StatusService);
};
