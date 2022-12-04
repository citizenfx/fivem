import { injectable, interfaces, multiInject, optional } from "inversify";
import { defineService, ServicesContainer } from "../../../base/servicesContainer";
import { AnalyticsProvider } from "./analytics.extensions";
import { IAnalyticsEvent } from "./types";

export const IAnalyticsService = defineService<IAnalyticsService>('AnalyticsService');
export interface IAnalyticsService {
  trackEvent(event: IAnalyticsEvent): void;
}

export function registerAnalyticsService(container: ServicesContainer, providers: interfaces.Newable<AnalyticsProvider>[] = []) {
  container.registerImpl(IAnalyticsService, AnalyticsService);

  providers.forEach((provider) => {
    container.registerImpl(AnalyticsProvider, provider);
  });
}

@injectable()
class AnalyticsService implements IAnalyticsService {
  @multiInject(AnalyticsProvider) @optional()
  protected readonly providers: AnalyticsProvider[];

  trackEvent(event: IAnalyticsEvent) {
    this.providers.forEach((provider) => provider.trackEvent(event));
  }
}
