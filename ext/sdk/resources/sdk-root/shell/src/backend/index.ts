import 'reflect-metadata';

console.log('FxDK Shell Backend is now loading');

(Error as any).prepareStackTrace = (error: Error) => error.stack?.toString();

process.on('uncaughtException', (error) => {
  console.log('UNHANDLED EXCEPTION', error);
  process.exit(-1);
});

process.on('unhandledRejection', (error) => {
  console.log('UNHANDLED REJECTION', error);
  process.exit(-1);
});

import { Container } from 'inversify';
import { ConfigService } from './config-service';
import { AppService } from './app/app-service';
import { bindShellBackend } from './shell-backend';
import { bindFs } from './fs/fs-bindings';
import { bindApi } from './api/api-bindings';
import { bindFeatures } from './features/features-bindings';
import { bindNotification } from './notification/notification-bindings';
import { bindExplorer } from './explorer/explorer-bindings';
import { bindApp } from './app/app-bindings';
import { bindStatus } from './status/status-bindings';
import { bindUpdater } from './updater/updater-bindings';
import { bindLogger } from './logger/logger-bindings';
import { bindTheia } from './theia/theia-bindings';
import { bindGameServer } from './game-server/game-server-bindings';
import { bindProject } from './project/project-bindings';
import { ContainerAccess } from './container-access';
import { bindTask } from './task/task-bindings';

const appContainer = new Container();

const configService = new ConfigService();
appContainer.bind(ConfigService).toConstantValue(configService);
appContainer.bind(ContainerAccess).toConstantValue(new ContainerAccess(appContainer));

bindApp(appContainer);
bindFeatures(appContainer);
bindFs(appContainer);
bindTask(appContainer);
bindStatus(appContainer);
bindExplorer(appContainer);
bindNotification(appContainer);
bindLogger(appContainer);
bindShellBackend(appContainer);
bindApi(appContainer);
bindGameServer(appContainer);
bindUpdater(appContainer);
bindTheia(appContainer);
bindProject(appContainer);


appContainer.get(AppService).startContributions().catch((e) => {
  console.error('Failed to start app contributions', e.message, e.stack);
});


setTimeout(() => {
  emit('sdk:startGame');
}, 2500);
