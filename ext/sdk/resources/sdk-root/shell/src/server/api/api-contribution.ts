import { inject, injectable, interfaces, named } from "inversify";
import { ApiClient } from "server/api/api-client";
import { AppContribution } from "server/app/app-contribution";
import { ContributionProvider } from "server/contribution-provider";
import { LogService } from "server/logger/log-service";
import { ClientEventBinding, getClientEventHandlers, getSystemEventHandlers, SystemEventBinding } from "./api-decorators";
import { systemEvents } from "../systemEvents";

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
    this.logService.log('\nInitializing api contribution', contribution.getId());

    // binding to client events
    const clientEventHandlers = getClientEventHandlers(contribution);
    this.logService.log('client event handlers', clientEventHandlers);

    const clientEventDisposers = clientEventHandlers
      .filter(({ propKey }) => !!contribution[propKey])
      .map(({ propKey, eventName }: ClientEventBinding) => this.apiClient.on(eventName, contribution[propKey].bind(contribution)));

    // binding to system events
    const systemEventHandlers = getSystemEventHandlers(contribution);
    this.logService.log('system event handlers', systemEventHandlers);

    const systemEventDisposers = systemEventHandlers
      .filter(({ propKey }) => !!contribution[propKey])
      .map(({ propKey, event }: SystemEventBinding) => systemEvents.on(event, contribution[propKey].bind(contribution)));

    if (Array.isArray(contribution.eventDisposers)) {
      contribution.eventDisposers.push(...clientEventDisposers, ...systemEventDisposers);
    }
  }

  boot() {
    const apiContributions = this.apiContributions.getAll();

    this.logService.log(`Initializing ${apiContributions.length} api contributions...`);

    apiContributions.forEach((apiContribution) => {
      this.initContribution(apiContribution);
    });
  }
}

export const bindApiContribution = <T>(container: interfaces.Container, service: interfaces.ServiceIdentifier<T>) => {
  container.bind(ApiContribution).toService(service);
};
