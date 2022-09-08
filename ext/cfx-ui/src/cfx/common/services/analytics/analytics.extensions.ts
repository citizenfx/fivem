import { interfaces } from "inversify";
import { ServicesContainer } from "../../../base/servicesContainer";
import { IAnalyticsEvent } from "./types";

export const AnalyticsProvider = Symbol('AnalyticsProvider');
export interface AnalyticsProvider {
  trackEvent(event: IAnalyticsEvent): void;
}

export function registerAnalyticsProvider(container: ServicesContainer, provider: interfaces.Newable<AnalyticsProvider>) {
  container.registerImpl(AnalyticsProvider, provider);
}
