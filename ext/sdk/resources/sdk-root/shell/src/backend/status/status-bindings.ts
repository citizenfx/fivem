import { interfaces } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { StatusService } from "./status-service";

export const bindStatus = (container: interfaces.Container) => {
  container.bind(StatusService).toSelf().inSingletonScope();
  container.bind(ApiContribution).toService(StatusService);
};
