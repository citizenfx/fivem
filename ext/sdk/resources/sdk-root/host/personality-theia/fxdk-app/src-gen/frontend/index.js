// @ts-check
require('es6-promise/auto');
require('reflect-metadata');
const { Container } = require('inversify');
const { FrontendApplicationConfigProvider } = require('@theia/core/lib/browser/frontend-application-config-provider');
FrontendApplicationConfigProvider.set({
    "applicationName": "Eclipse Theia",
    "defaultTheme": "dark",
    "defaultIconTheme": "none"
});
const { FrontendApplication } = require('@theia/core/lib/browser');
const { frontendApplicationModule } = require('@theia/core/lib/browser/frontend-application-module');
const { messagingFrontendModule } = require('@theia/core/lib/browser/messaging/messaging-frontend-module');
const { loggerFrontendModule } = require('@theia/core/lib/browser/logger-frontend-module');
const { ThemeService } = require('@theia/core/lib/browser/theming');

const container = new Container();
container.load(frontendApplicationModule);
container.load(messagingFrontendModule);
container.load(loggerFrontendModule);

function load(raw) {
    return Promise.resolve(raw.default).then(module =>
        container.load(module)
    )
}

function start() {
    (window['theia'] = window['theia'] || {}).container = container;

    const themeService = ThemeService.get();
    themeService.loadUserTheme();

    const application = container.get(FrontendApplication);
    return application.start();
}

module.exports = Promise.resolve()
    .then(function () { return import('@theia/core/lib/browser/menu/browser-menu-module').then(load) })
    .then(function () { return import('@theia/core/lib/browser/window/browser-window-module').then(load) })
    .then(function () { return import('@theia/core/lib/browser/keyboard/browser-keyboard-module').then(load) })
    .then(function () { return import('@theia/variable-resolver/lib/browser/variable-resolver-frontend-module').then(load) })
    .then(function () { return import('@theia/editor/lib/browser/editor-frontend-module').then(load) })
    .then(function () { return import('@theia/filesystem/lib/browser/filesystem-frontend-module').then(load) })
    .then(function () { return import('@theia/filesystem/lib/browser/download/file-download-frontend-module').then(load) })
    .then(function () { return import('@theia/filesystem/lib/browser/file-dialog/file-dialog-module').then(load) })
    .then(function () { return import('@theia/workspace/lib/browser/workspace-frontend-module').then(load) })
    .then(function () { return import('@theia/navigator/lib/browser/navigator-frontend-module').then(load) })
    .then(function () { return import('@theia/markers/lib/browser/problem/problem-frontend-module').then(load) })
    .then(function () { return import('@theia/outline-view/lib/browser/outline-view-frontend-module').then(load) })
    .then(function () { return import('@theia/monaco/lib/browser/monaco-browser-module').then(load) })
    .then(function () { return import('@theia/callhierarchy/lib/browser/callhierarchy-frontend-module').then(load) })
    .then(function () { return import('@theia/console/lib/browser/console-frontend-module').then(load) })
    .then(function () { return import('@theia/output/lib/browser/output-frontend-module').then(load) })
    .then(function () { return import('@theia/userstorage/lib/browser/user-storage-frontend-module').then(load) })
    .then(function () { return import('@theia/preferences/lib/browser/preference-frontend-module').then(load) })
    .then(function () { return import('@theia/process/lib/common/process-common-module').then(load) })
    .then(function () { return import('@theia/terminal/lib/browser/terminal-frontend-module').then(load) })
    .then(function () { return import('@theia/task/lib/browser/task-frontend-module').then(load) })
    .then(function () { return import('@theia/debug/lib/browser/debug-frontend-module').then(load) })
    .then(function () { return import('@theia/editor-preview/lib/browser/editor-preview-frontend-module').then(load) })
    .then(function () { return import('@theia/file-search/lib/browser/file-search-frontend-module').then(load) })
    .then(function () { return import('@theia/keymaps/lib/browser/keymaps-frontend-module').then(load) })
    .then(function () { return import('@theia/getting-started/lib/browser/getting-started-frontend-module').then(load) })
    .then(function () { return import('@theia/scm/lib/browser/scm-frontend-module').then(load) })
    .then(function () { return import('@theia/scm-extra/lib/browser/scm-extra-frontend-module').then(load) })
    .then(function () { return import('@theia/git/lib/browser/git-frontend-module').then(load) })
    .then(function () { return import('@theia/git/lib/browser/prompt/git-prompt-module').then(load) })
    .then(function () { return import('@theia/messages/lib/browser/messages-frontend-module').then(load) })
    .then(function () { return import('@theia/mini-browser/lib/browser/mini-browser-frontend-module').then(load) })
    .then(function () { return import('@theia/search-in-workspace/lib/browser/search-in-workspace-frontend-module').then(load) })
    .then(function () { return import('@theia/timeline/lib/browser/timeline-frontend-module').then(load) })
    .then(function () { return import('@theia/plugin-ext/lib/plugin-ext-frontend-module').then(load) })
    .then(function () { return import('@theia/plugin-dev/lib/browser/plugin-dev-frontend-module').then(load) })
    .then(function () { return import('@theia/plugin-ext-vscode/lib/browser/plugin-vscode-frontend-module').then(load) })
    .then(function () { return import('@theia/plugin-metrics/lib/browser/plugin-metrics-frontend-module').then(load) })
    .then(function () { return import('@theia/preview/lib/browser/preview-frontend-module').then(load) })
    .then(function () { return import('@theia/typehierarchy/lib/browser/typehierarchy-frontend-module').then(load) })
    .then(function () { return import('@theia/vsx-registry/lib/browser/vsx-registry-frontend-module').then(load) })
    .then(function () { return import('fxdk-game-view/lib/browser/fxdk-game-view-frontend-module').then(load) })
    .then(function () { return import('fxdk-project/lib/browser/fxdk-project-frontend-module').then(load) })
    .then(start).catch(reason => {
        console.error('Failed to start the frontend application.');
        if (reason) {
            console.error(reason);
        }
    });