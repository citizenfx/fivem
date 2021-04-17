import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { bindContributionProvider } from "backend/contribution-provider";
import { bindLogProvider } from "backend/logger/log-provider";
import { ApiClient } from "./api-client";
import { ApiClientLogger } from "./api-client-logger";
import { ApiContribution, ApiContributionFactory, ApiService } from "./api-contribution";
import { ApiClientScoped } from "./api-client-scoped";

export const bindApi = (container: interfaces.Container) => {
  container.bind(ApiClient).toSelf().inSingletonScope();

  container.bind(ApiClientScoped).toDynamicValue((ctx) => ctx.container.resolve(ApiClientScoped));

  container.bind(ApiClientLogger).toSelf().inSingletonScope();
  bindLogProvider(container, ApiClientLogger);

  container.bind(ApiService).toSelf().inSingletonScope();
  bindAppContribution(container, ApiService);

  container.bind(ApiContributionFactory).toFactory((ctx: interfaces.Context) => <T extends ApiContribution>(service: interfaces.Newable<T>) => {
    const apiContribution = ctx.container.resolve(service);
    const apiService = ctx.container.get(ApiService);

    apiService.initContribution(apiContribution);

    return apiContribution;
  });

  bindContributionProvider(container, ApiContribution);
};
