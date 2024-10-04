import { injectable, interfaces, multiInject, optional } from 'inversify';
import React from 'react';
import { useLocation } from 'react-router-dom';

import { AnalyticsProvider } from './analytics.extensions';
import { IAnalyticsEvent, TrackEventParams } from './types';
import { defineService, ServicesContainer, useService } from '../../../base/servicesContainer';

export const IAnalyticsService = defineService<IAnalyticsService>('AnalyticsService');
export interface IAnalyticsService {
  trackEvent(event: IAnalyticsEvent): void;
}

export function registerAnalyticsService(
  container: ServicesContainer,
  providers: interfaces.Newable<AnalyticsProvider>[] = [],
) {
  container.registerImpl(IAnalyticsService, AnalyticsService);

  providers.forEach((provider) => {
    container.registerImpl(AnalyticsProvider, provider);
  });
}

@injectable()
class AnalyticsService implements IAnalyticsService {
  @multiInject(AnalyticsProvider)
  @optional()
  protected readonly providers: AnalyticsProvider[];

  trackEvent(event: IAnalyticsEvent) {
    this.providers.forEach((provider) => provider.trackEvent(event));
  }
}

export function useAnalyticsService(): IAnalyticsService {
  return useService(IAnalyticsService);
}

function getViewName(pathname: string) {
  if (pathname === '/') {
    return 'home';
  }

  // Remove server id from path
  if (pathname.includes('/servers/detail/')) {
    return 'server_detail';
  }

  return pathname.split('/').filter(Boolean).join('_');
}

export function useEventHandler() {
  const analyticsService = useAnalyticsService();
  // the easiest way to get info aboute page is a hook useLocation
  // thats why prefer to use useEventHandler in components
  const location = useLocation();

  const eventHandler = React.useCallback(
    (params: TrackEventParams) => {
      const enhancedProperties = {
        view_name: getViewName(location.pathname),
        ...params.properties,
      };

      analyticsService.trackEvent({
        action: params.action,
        properties: enhancedProperties,
      });
    },
    [analyticsService, location],
  );

  return eventHandler;
}
