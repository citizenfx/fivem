import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import * as serviceWorker from './serviceWorker';
import './common/game-view.webcomponent';
import { Shell } from './components/Shell';

ReactDOM.render(
  <React.StrictMode>
    <Shell />
  </React.StrictMode>,
  document.getElementById('root')
);

serviceWorker.unregister();
