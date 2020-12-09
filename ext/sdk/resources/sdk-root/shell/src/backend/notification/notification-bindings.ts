import { interfaces } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { AppContribution } from "backend/app/app-contribution";
import { NotificationService } from "./notification-service";

export const bindNotification = (container: interfaces.Container) => {
  container.bind(NotificationService).toSelf().inSingletonScope();

  container.bind(AppContribution).toService(NotificationService);
  container.bind(ApiContribution).toService(NotificationService);
};
