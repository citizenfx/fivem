import './bootstrap';
import '@fontsource/montserrat/300.css';
import '@fontsource/montserrat/400.css';
import '@fontsource/rubik/variable.css';
import '@fontsource/rubik/variable-italic.css';
import '@fontsource/source-code-pro/300.css';
import './global.scss';

import './shell.main';

import React from 'react';
import ReactDOM from 'react-dom';
import { TitleManager } from 'fxdk/browser/managers/TitleManager';
import { ConfirmationManager } from 'fxdk/browser/managers/ConfirmationManager';
import { ShellLifecycle } from 'fxdk/browser/shellLifecycle';
import { observer } from 'mobx-react-lite';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { renderViewRegistryItems } from 'fxdk/base/viewRegistry';
import s from './Shell.module.scss';

const Shell = observer(function Shell() {
  const participantNodes = renderViewRegistryItems(ShellViewParticipants.getAllVisible());

  return (
    <div className={s.root}>
      {participantNodes}
    </div>
  );
});

ShellLifecycle.advancePhase();

ReactDOM.render(
  <React.StrictMode>
    <TitleManager />
    <ConfirmationManager />

    <Shell />

    <div className="resize-sentinel" />
  </React.StrictMode>,
  document.getElementById('root')
);

console.timeEnd('Shell initial render took:');

requestIdleCallback(() => ShellLifecycle.advancePhase());
