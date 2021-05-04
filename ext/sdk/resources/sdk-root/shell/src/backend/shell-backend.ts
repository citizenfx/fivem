import cors from 'cors';
import express from 'express';
import expressWs from 'express-ws';
import { Container, inject, injectable } from 'inversify';
import { ConfigService } from './config-service';
import { SingleEventEmitter } from '../utils/singleEventEmitter';
import { LogService } from './logger/log-service';
import { AppContribution, bindAppContribution } from './app/app-contribution';
import { Deferred } from './deferred';

@injectable()
export class ShellBackend implements AppContribution {
  public readonly onStarted = new SingleEventEmitter<void>();

  public readonly expressApp: expressWs.Application;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(LogService)
  protected readonly logService: LogService;

  constructor() {
    this.expressApp = (express() as any);

    // Wrapping with WebSockets support
    expressWs(this.expressApp);
  }

  boot() {
    const expressAppBootComplete = new Deferred();

    // Applying CORS
    this.expressApp.use(cors({ origin: '*' }));

    // Setting up static folder to serve
    this.expressApp.use(express.static(this.configService.sdkRootShellBuild));

    this.expressApp.listen(this.configService.shellBackendPort, () => {
      this.logService.log(`Shell backend is listening on http://localhost:${this.configService.shellBackendPort}`);

      expressAppBootComplete.resolve();
    });

    return expressAppBootComplete.promise;
  }
}

export const bindShellBackend = (container: Container) => {
  container.bind(ShellBackend).toSelf().inSingletonScope();
  bindAppContribution(container, ShellBackend);
};
