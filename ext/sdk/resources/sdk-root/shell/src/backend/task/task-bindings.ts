import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { TaskReporterService } from "./task-reporter-service";
import { bindAppContribution } from "backend/app/app-contribution";

export const bindTask = (container: interfaces.Container) => {
  container.bind(TaskReporterService).toSelf().inSingletonScope();
  bindApiContribution(container, TaskReporterService);
  bindAppContribution(container, TaskReporterService);
};
