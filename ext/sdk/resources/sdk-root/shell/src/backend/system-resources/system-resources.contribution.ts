import { registerAppContribution } from "backend/app/app.extensions";
import { SystemResourcesService } from "./system-resources-service";
import { registerSingleton } from "backend/container-access";

registerAppContribution(
  registerSingleton(SystemResourcesService)
);
