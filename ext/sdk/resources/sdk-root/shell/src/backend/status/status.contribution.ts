import { registerApiContribution } from "backend/api/api.extensions";
import { StatusService } from "./status-service";
import { registerAppContribution } from "backend/app/app.extensions";
import { registerSingleton } from "backend/container-access";

registerApiContribution(
  registerAppContribution(
    registerSingleton(StatusService)
  )
);
