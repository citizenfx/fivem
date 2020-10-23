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

ReactDOM.render(
  <React.StrictMode>
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
  </React.StrictMode>,
  document.getElementById('root')
);

serviceWorker.unregister();

openDevTools();

document.addEventListener('contextmenu', (event) => {
  event.preventDefault();
});
