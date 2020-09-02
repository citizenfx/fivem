import React from 'react';
import ReactDOM from 'react-dom';
import './index.css';
import * as serviceWorker from './serviceWorker';
import { RootApp } from './components/RootApp';

ReactDOM.render(
  <React.StrictMode>
    <RootApp />
  </React.StrictMode>,
  document.getElementById('root')
);

serviceWorker.unregister();
