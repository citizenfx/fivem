import { enableLogger } from 'utils/logger';
import { IWindowWithShellApi } from "shell-api";
import { ShellEvents } from "shell-api/events";
import * as Sentry from "@sentry/react";
import { Integrations } from "@sentry/tracing";
import { stateApi } from 'shared/api.events';
import { ShellCommands } from 'shell-api/commands';
import { Api } from './Api';

// window.openDevTools();

enableLogger('shell,shell:*,host');

(window as IWindowWithShellApi).shellApi = {
  events: ShellEvents,
  commands: ShellCommands,
};

document.addEventListener('click', (event: MouseEvent) => {
  const target: HTMLElement = event.target as any;

  const isA = target.matches('a');
  const isAChild = target.matches('a *');

  if (!isA && !isAChild) {
    return;
  }

  event.preventDefault();

  const link = isA
    ? target.getAttribute('href')
    : target.closest('a')?.getAttribute('href');


  if (link) {
    invokeNative('openUrl', link);
  }
});

document.addEventListener('contextmenu', (event) => {
  event.preventDefault();
});

if (process.env.CI_PIPELINE_ID !== 'dev') {
  Sentry.init({
    dsn: "https://e3b160e20aa24ffd9b74a222a4d5c07a@sentry.fivem.net/7",
    release: `cfx-${process.env.CI_PIPELINE_ID}`,
    integrations: [
      new Integrations.BrowserTracing(),
    ],
    tracesSampleRate: 1.0,
  });
}

if (!initRGDInput()) {
  throw new Error('Failed to init RGD input');
}

Api.on(stateApi.setUserId, (id: string) => Sentry.setUser({ id }));
