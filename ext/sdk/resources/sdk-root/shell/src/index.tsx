import 'typeface-montserrat';
import 'typeface-rubik';
import './index.scss';

import React from 'react';
import ReactDOM from 'react-dom';
import * as Sentry from "@sentry/react";
import { Integrations } from "@sentry/tracing";
import { Shell } from './components/Shell';
import { enableLogger } from 'utils/logger';
import { StateContextProvider } from './contexts/StateContext';
import { ProjectContextProvider } from './contexts/ProjectContext';
import { TheiaContextProvider } from './contexts/TheiaContext';
import { ServerContextProvider } from './contexts/ServerContext';
import { StatusContextProvider } from 'contexts/StatusContext';
import { TaskContextProvider } from 'contexts/TaskContext';
import { TitleManager } from 'managers/TitleManager';
import { TheiaProjectManager } from 'managers/TheiaProjectManager';
import { NotificationsManager } from 'managers/NotificationsManager/NotificationsManager';
import { OutputContextProvider } from 'contexts/OutputContext';
import { GameContextProvider } from 'contexts/GameContext';
import { SdkMessageManager } from 'managers/SdkMessageManager';
import { GameConnectionManager } from 'managers/GameConnectionManager';
import { ConsolesManager } from 'managers/ConsolesManager';
import { TheiaCommandsManager } from 'managers/TheiaCommandsManager';

enableLogger('shell,shell:*,host');

Sentry.init({
  dsn: "https://e3b160e20aa24ffd9b74a222a4d5c07a@sentry.fivem.net/7",
  release: `cfx-${process.env.CI_PIPELINE_ID}`,
  integrations: [
    new Integrations.BrowserTracing(),
  ],
  tracesSampleRate: 1.0,
});

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
    <GameContextProvider>
      <OutputContextProvider>
        <TaskContextProvider>
          <StatusContextProvider>
            <StateContextProvider>
              <TheiaContextProvider>
                <ProjectContextProvider>
                  <ServerContextProvider>
                    <SdkMessageManager />
                    <GameConnectionManager />
                    <TitleManager />
                    <TheiaProjectManager />
                    <TheiaCommandsManager />
                    <NotificationsManager />
                    <ConsolesManager />

                    <Shell />
                  </ServerContextProvider>
                </ProjectContextProvider>
              </TheiaContextProvider>
            </StateContextProvider>
          </StatusContextProvider>
        </TaskContextProvider>
      </OutputContextProvider>
    </GameContextProvider>
  </React.StrictMode>,
  document.getElementById('root')
);
