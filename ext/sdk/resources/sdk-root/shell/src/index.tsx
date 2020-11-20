import 'typeface-montserrat';
import 'typeface-rubik';
import './index.scss';

import React from 'react';
import ReactDOM from 'react-dom';
import { Shell } from './components/Shell';
import { StateContextProvider } from './contexts/StateContext';
import { ProjectContextProvider } from './contexts/ProjectContext';
import { TheiaContextProvider } from './contexts/TheiaContext';
import { ServerContextProvider } from './contexts/ServerContext';
import { StatusContextProvider } from 'contexts/StatusContext';
import { TitleManager } from 'managers/TitleManager';
import { TheiaProjectManager } from 'managers/TheiaProjectManager';
import { enableLogger } from 'utils/logger';

enableLogger('shell,shell:*,host');

ReactDOM.render(
  <React.StrictMode>
    <StatusContextProvider>
      <StateContextProvider>
        <TheiaContextProvider>
          <ProjectContextProvider>
            <ServerContextProvider>
              <TitleManager />
              <TheiaProjectManager />
              <Shell />
            </ServerContextProvider>
          </ProjectContextProvider>
        </TheiaContextProvider>
      </StateContextProvider>
    </StatusContextProvider>
  </React.StrictMode>,
  document.getElementById('root')
);

document.addEventListener('contextmenu', (event) => {
  event.preventDefault();
});
