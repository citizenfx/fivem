import 'typeface-montserrat';
import 'typeface-rubik';
import './index.scss';

import React from 'react';
import ReactDOM from 'react-dom';
import * as Sentry from "@sentry/react";
import { Integrations } from "@sentry/tracing";
import { Shell } from './components/Shell';
import { enableLogger } from 'utils/logger';
import { TitleManager } from 'managers/TitleManager';
import { TheiaProjectManager } from 'managers/TheiaProjectManager';
import { onApiMessage } from 'utils/api';
import { stateApi } from 'shared/api.events';

enableLogger('shell,shell:*,host');

// window.openDevTools();

// try {
//   (CSS as any).paintWorklet.addModule('paintlet.js');
// } catch (e) {
//   console.error('No paintlets for you :<', e);
// }

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

onApiMessage(stateApi.setUserId, (id: string) => Sentry.setUser({ id }));

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

ReactDOM.render(
  <React.StrictMode>
    <TitleManager />
    <TheiaProjectManager />

    <Shell />

    <div className="resize-sentinel" />
  </React.StrictMode>,
  document.getElementById('root')
);
