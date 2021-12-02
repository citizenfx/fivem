import { registerApiContribution } from "backend/api/api.extensions";
import { registerContributionProvider } from "backend/contribution-provider";
import { AppContribution, registerAppContribution } from "./app.extensions";
import { AppService } from "./app-service";
import { AppServiceFactory } from "./app-service-factory";
import { AppStateService } from "./app-state-service";
import { registerFactory, registerSingleton } from "backend/container-access";

registerSingleton(AppService);

registerApiContribution(
  registerAppContribution(
    registerSingleton(AppStateService)
  )
);

registerContributionProvider(AppContribution);
registerFactory(AppServiceFactory, (ctx) => (service) => ctx.container.resolve(service));
