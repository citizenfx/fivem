import { interfaces } from 'inversify';

import { IAnalyticsEvent } from './types';
import { ServicesContainer } from '../../../base/servicesContainer';

export const AnalyticsProvider = Symbol('AnalyticsProvider');
export interface AnalyticsProvider {
  trackEvent(event: IAnalyticsEvent): void;
}

export function registerAnalyticsProvider(
  container: ServicesContainer,
  provider: interfaces.Newable<AnalyticsProvider>,
) {
  container.registerImpl(AnalyticsProvider, provider);
}
