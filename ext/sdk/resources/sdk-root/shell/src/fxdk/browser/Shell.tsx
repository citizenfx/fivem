import './bootstrap';
import '@fontsource/montserrat/300.css';
import '@fontsource/montserrat/400.css';
import '@fontsource/rubik/variable.css';
import '@fontsource/rubik/variable-italic.css';
import '@fontsource/source-code-pro/300.css';
import './global.scss';

import './browser.main';

import React from 'react';
import ReactDOM from 'react-dom';
import { ConfirmationManager } from 'fxdk/browser/managers/ConfirmationManager';
import { ShellLifecycle } from 'fxdk/browser/shellLifecycle';
import { observer } from 'mobx-react-lite';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { renderViewRegistryItems } from 'fxdk/base/viewRegistry';
import { reaction } from 'mobx';
import { Project } from 'fxdk/project/browser/state/project';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';
import { WEState } from 'personalities/world-editor/store/WEState';
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
    <ConfirmationManager />

    <Shell />

    <div className="resize-sentinel" />
  </React.StrictMode>,
  document.getElementById('root')
);

console.timeEnd('Shell initial render took:');

requestIdleCallback(() => ShellLifecycle.advancePhase());

const titleBase = 'Cfx.re Development Kit (FiveM)';
const titleNode: HTMLTitleElement = document.querySelector('head>title')!;
reaction(
  () => {
    const parts = [titleBase];

    if (WEState.mapName) {
      parts.push(WEState.mapName);
    }

    if (ProjectLoader.hasProject) {
      parts.push(Project.name);
    }

    return parts.reverse().join(' / ');
  },
  (title) => {
    titleNode.innerText = title;
  },
);
