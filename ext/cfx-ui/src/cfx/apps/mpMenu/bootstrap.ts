import 'reflect-metadata';

// Styles setup
import '@fontsource/montserrat/variable.css';
import '@fontsource/montserrat/variable-italic.css';
import '@fontsource/rubik/variable.css';
import '@fontsource/rubik/variable-italic.css';

import './styles/index.scss';
import 'cfx/styles/global.scss';

import { isExternalUrl } from 'cfx/utils/links';

import { mpMenu } from './mpMenu';

// Install all anchors click intercepter to properly handle clicks on external links
document.addEventListener('click', (event: MouseEvent) => {
  const target: HTMLElement = event.target as any;

  const isA = target.matches('a');
  const isAChild = target.matches('a *');

  if (!isA && !isAChild) {
    return;
  }

  const link = isA
    ? target.getAttribute('href')
    : target.closest('a')?.getAttribute('href');

  if (link && isExternalUrl(link)) {
    event.preventDefault();
    mpMenu.invokeNative('openUrl', link);
  }
});
