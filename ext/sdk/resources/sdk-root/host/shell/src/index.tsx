import React from 'react';
import ReactDOM from 'react-dom';
import './index.scss';
import * as serviceWorker from './serviceWorker';
import './common/game-view.webcomponent';
import { Shell } from './components/Shell';
import { StateProvider } from './components/State';

ReactDOM.render(
  <React.StrictMode>
    <StateProvider>
      <Shell />
    </StateProvider>
  </React.StrictMode>,
  document.getElementById('root')
);

serviceWorker.unregister();
