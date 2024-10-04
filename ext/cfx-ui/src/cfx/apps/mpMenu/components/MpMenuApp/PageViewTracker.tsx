/* eslint-disable react-hooks/rules-of-hooks */
import React from 'react';
import { useLocation } from 'react-router-dom';

import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames } from 'cfx/common/services/analytics/types';

export function NavigationTracker() {
  const location = useLocation();
  const eventHandler = useEventHandler();

  React.useEffect(() => {
    try {
      if (__CFXUI_DEV__) {
        console.log('Tracking page view', location.pathname);
      }

      eventHandler({
        action: EventActionNames.PageViews,
        properties: { position: undefined },
      });
    } catch (e) {
      // noop
    }
  }, [location]);

  return null;
}
