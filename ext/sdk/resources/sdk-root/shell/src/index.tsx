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

enableLogger('shell,shell:*,host');

ReactDOM.render(
  <React.StrictMode>
    <OutputContextProvider>
      <TaskContextProvider>
        <StatusContextProvider>
          <StateContextProvider>
            <TheiaContextProvider>
              <ProjectContextProvider>
                <ServerContextProvider>
                  <TitleManager />
                  <TheiaProjectManager />
                  <NotificationsManager />

                  <Shell />
                </ServerContextProvider>
              </ProjectContextProvider>
            </TheiaContextProvider>
          </StateContextProvider>
        </StatusContextProvider>
      </TaskContextProvider>
    </OutputContextProvider>
  </React.StrictMode>,
  document.getElementById('root')
);

document.addEventListener('contextmenu', (event) => {
  event.preventDefault();
});
