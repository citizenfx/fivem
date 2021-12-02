import { registerApiContribution } from "backend/api/api.extensions";
import { TaskReporterService } from "./task-reporter-service";
import { registerAppContribution } from "backend/app/app.extensions";
import { registerSingleton } from "backend/container-access";

registerApiContribution(
  registerAppContribution(
    registerSingleton(TaskReporterService)
  )
);
