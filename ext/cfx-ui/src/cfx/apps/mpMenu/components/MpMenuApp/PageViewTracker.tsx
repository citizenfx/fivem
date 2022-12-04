import React from "react";
import { useLocation } from "react-router-dom";

export function NavigationTracker() {
  if (__CFXUI_DEV__) {
    const location = useLocation();

    React.useEffect(() => {
      try {
        console.log('Tracking page view', location.pathname);
        _paq.push(['setCustomUrl', '/ui/app' + location.pathname]);
        _paq.push(['trackPageView']);
      } catch (e) {
        // noop
      }
    }, [location]);
  }

  return null;
}
