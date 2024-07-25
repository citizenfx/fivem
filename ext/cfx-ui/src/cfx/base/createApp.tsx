import { noop } from '@cfx-dev/ui-components';
import React from 'react';
import { createRoot } from 'react-dom/client';

import { ServicesContainer, ServicesContainerContext } from 'cfx/base/servicesContainer';
import { getAppService, registerAppService } from 'cfx/common/services/app/app.service';

export interface AppDefinition {
  defineServices(container: ServicesContainer): void;
  render(): React.ReactNode;

  appNodeSelector?: string;

  beforeRender?(container: ServicesContainer): void | Promise<void>;
  afterRender?(container: ServicesContainer): void | Promise<void>;
}

export async function startBrowserApp(definition: AppDefinition) {
  const {
    defineServices,
    render,
    appNodeSelector = '#cfxui-root',

    beforeRender = noop,
    afterRender = noop,
  } = definition;

  const $container = document.querySelector(appNodeSelector);

  if (!$container) {
    document.body.style.display = 'flex';
    document.body.style.alignItems = 'center';
    document.body.style.justifyContent = 'center';
    document.body.style.backgroundColor = 'black';
    document.body.style.color = 'red';

    // eslint-disable-next-line @stylistic/max-len
    document.body.innerHTML = `<h1>Invalid HTML template, no ${appNodeSelector} node available, CFXUI app will not be rendered</h1>`;

    return;
  }

  const container = new ServicesContainer(defineServices);

  registerAppService(container);

  const appService = getAppService(container);

  const reactRoot = createRoot($container);

  await beforeRender(container);
  await appService.beforeRender();

  function AfterRender() {
    // Using layout effect instead of normal effect as layout effect triggers after vdom committed to the dom
    // Otherwise we trigger afterRender way too soon and animations have almost 100% chance to stutter
    React.useLayoutEffect(() => {
      let shouldClearTimer = true;

      // Reschedule for next tick to unblock paint and composite browser routines
      const timer = setTimeout(() => {
        shouldClearTimer = false;

        appService.afterRender();
        afterRender(container);
      }, 0);

      return () => {
        if (shouldClearTimer) {
          clearTimeout(timer);
        }
      };
    }, []);

    return null;
  }

  reactRoot.render(
    <ServicesContainerContext.Provider value={container}>
      {render()}

      <AfterRender />
    </ServicesContainerContext.Provider>,
  );
}
