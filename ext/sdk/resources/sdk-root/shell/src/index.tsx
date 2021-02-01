import 'typeface-montserrat';
import 'typeface-rubik';
import './index.scss';

import React from 'react';
import ReactDOM from 'react-dom';
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

enableLogger('shell,shell:*,host');

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

document.addEventListener('contextmenu', (event) => {
  event.preventDefault();
});
