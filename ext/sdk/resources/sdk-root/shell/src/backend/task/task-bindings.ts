import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { TaskReporterService } from "./task-reporter-service";

export const bindTask = (container: interfaces.Container) => {
  container.bind(TaskReporterService).toSelf().inSingletonScope();
  bindApiContribution(container, TaskReporterService);
};
