import React from "react";
import { getAppService, registerAppService } from "cfx/common/services/app/app.service";
import { ServicesContainer, ServicesContainerContext } from "cfx/base/servicesContainer";
import { createRoot } from "react-dom/client";
import { noop } from "cfx/utils/functional";

export interface AppDefinition {
  defineServices(container: ServicesContainer): void,
  render(): React.ReactNode,

  appNodeSelector?: string,

  beforeRender?(container: ServicesContainer): void | Promise<void>,
  afterRender?(container: ServicesContainer): void | Promise<void>,
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

    document.body.innerHTML = '<h1>Invalid HTML template, no #cfxui-root node available, CFXUI app will not be rendered</h1>';

    return;
  }

  const container = new ServicesContainer(defineServices);

  registerAppService(container);

  const appService = getAppService(container);

  const reactRoot = createRoot($container);

  await beforeRender(container);
  await appService.beforeRender();

  function AfterRender() {
    React.useEffect(() => {
      setTimeout(() => {
        afterRender(container);
        appService.afterRender();
      }, 1000);
    }, []);

    return null;
  }

  reactRoot.render(
    <ServicesContainerContext.Provider value={container}>
      {render()}

      <AfterRender />
    </ServicesContainerContext.Provider>
  );
}
