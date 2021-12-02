import { registerApiContribution } from "backend/api/api.extensions";
import { registerAppContribution } from "backend/app/app.extensions";
import { NotificationService } from "./notification-service";
import { registerSingleton } from "backend/container-access";

registerApiContribution(
  registerAppContribution(
    registerSingleton(NotificationService)
  )
);
