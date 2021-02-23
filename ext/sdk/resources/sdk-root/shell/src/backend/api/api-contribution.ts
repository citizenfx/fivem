import { inject, injectable, interfaces, named } from "inversify";
import { ApiClient } from "backend/api/api-client";
import { AppContribution } from "backend/app/app-contribution";
import { ContributionProvider } from "backend/contribution-provider";
import { LogService } from "backend/logger/log-service";
import { ClientEventBinding, getClientCallbackEventHandlers, getClientEventHandlers } from "./api-decorators";

export const ApiContributionFactory = Symbol('Factory<ApiContribution>');
export type ApiContributionFactory = <T extends ApiContribution>(service: interfaces.Newable<T>) => T;

export const ApiContribution = Symbol('ApiContribution');
export interface ApiContribution {
  eventDisposers?: Function[],

  getId(): string,
}

@injectable()
export class ApiService implements AppContribution {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ContributionProvider) @named(ApiContribution)
  protected readonly apiContributions: ContributionProvider<ApiContribution>;

  initContribution(contribution: ApiContribution) {
    // binding to client events
    const clientEventHandlers = getClientEventHandlers(contribution);

    const clientEventDisposers = clientEventHandlers
      .filter(({ propKey }) => !!contribution[propKey])
      .map(({ propKey, eventName }: ClientEventBinding) => this.apiClient.on(eventName, contribution[propKey].bind(contribution)));

    // binding to client callback events
    const clientCallbackEventHandlers = getClientCallbackEventHandlers(contribution);
    const clientCallbackEventDisposers = clientCallbackEventHandlers
      .filter(({ propKey }) => !!contribution[propKey])
      .map(({ propKey, eventName }: ClientEventBinding) => this.apiClient.onCallback(eventName, contribution[propKey].bind(contribution)));

    if (Array.isArray(contribution.eventDisposers)) {
      contribution.eventDisposers.push(...clientEventDisposers);
      contribution.eventDisposers.push(...clientCallbackEventDisposers);
    }
  }

  boot() {
    const apiContributions = this.apiContributions.getAll();

    apiContributions.forEach((apiContribution) => {
      this.initContribution(apiContribution);
    });
  }
}

export const bindApiContribution = <T>(container: interfaces.Container, service: interfaces.ServiceIdentifier<T>) => {
  container.bind(ApiContribution).toService(service);
};
