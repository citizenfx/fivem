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

if (process.env.__CFX_SENTRY_DSN__ && process.env.__CFX_SENTRY_RELEASE__) {
  Sentry.init({
    dsn: process.env.__CFX_SENTRY_DSN__,
    release: process.env.__CFX_SENTRY_RELEASE__,
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
