import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { bindContributionProvider } from "backend/contribution-provider";
import { AppContribution, bindAppContribution } from "./app-contribution";
import { AppService } from "./app-service";
import { AppServiceFactory } from "./app-service-factory";
import { AppStateService } from "./app-state-service";

export const bindApp = (container: interfaces.Container) => {
  container.bind(AppService).toSelf().inSingletonScope();

  container.bind(AppStateService).toSelf().inSingletonScope();
  bindApiContribution(container, AppStateService);
  bindAppContribution(container, AppStateService);

  bindContributionProvider(container, AppContribution);

  container.bind(AppServiceFactory).toFactory((ctx: interfaces.Context): AppServiceFactory => (service) => {
    return ctx.container.resolve(service);
  });
};
