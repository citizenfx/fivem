import { interfaces } from "inversify";
import { registerAppContribution } from "backend/app/app.extensions";
import { registerContributionProvider } from "backend/contribution-provider";
import { registerLogProvider } from "backend/logger/log-provider";
import { ApiClient } from "./api-client";
import { ApiClientLogger } from "./api-client-logger";
import { ApiContribution, ApiContributionFactory, ApiService } from "./api.extensions";
import { ApiClientScoped } from "./api-client-scoped";
import { registerDynamic, registerFactory, registerSingleton } from "backend/container-access";

registerSingleton(ApiClient);
registerDynamic(ApiClientScoped, (ctx) => ctx.container.resolve(ApiClientScoped));

registerLogProvider(registerSingleton(ApiClientLogger));
registerAppContribution(registerSingleton(ApiService));

registerFactory(ApiContributionFactory, (ctx) => <T extends ApiContribution>(service: interfaces.Newable<T>) => {
  const apiContribution = ctx.container.resolve(service);
  const apiService = ctx.container.get(ApiService);

  apiService.initContribution(apiContribution);

  return apiContribution;
});

registerContributionProvider(ApiContribution);
