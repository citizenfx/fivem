import 'typeface-montserrat';
import 'typeface-rubik';
import './index.scss';

import React from 'react';
import ReactDOM from 'react-dom';
import * as serviceWorker from './serviceWorker';
import './common/game-view.webcomponent';
import { Shell } from './components/Shell';
import { StateContextProvider } from './contexts/StateContext';
import { ProjectContextProvider } from './contexts/ProjectContext';
import { TheiaContextProvider } from './contexts/TheiaContext';
import { TitleManager } from './TitleManager';
import { ServerContextProvider } from './contexts/ServerContext';
import { StatusContextProvider } from 'contexts/StatusContext';

ReactDOM.render(
  <React.StrictMode>
    <StatusContextProvider>
      <StateContextProvider>
        <TheiaContextProvider>
          <ProjectContextProvider>
            <ServerContextProvider>
              <TitleManager />
              <Shell />
            </ServerContextProvider>
          </ProjectContextProvider>
        </TheiaContextProvider>
      </StateContextProvider>
    </StatusContextProvider>
  </React.StrictMode>,
  document.getElementById('root')
);

serviceWorker.unregister();

document.addEventListener('contextmenu', (event) => {
  event.preventDefault();
});
