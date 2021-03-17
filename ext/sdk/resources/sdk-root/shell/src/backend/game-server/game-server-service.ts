import fs from 'fs';
import cp from 'child_process';
import net from 'net';
import byline from 'byline';
import { inject, injectable } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { AppContribution } from "backend/app/app-contribution";
import { ServerStates, ServerUpdateChannel } from 'shared/api.types';
import { handlesClientEvent } from 'backend/api/api-decorators';
import { serverApi } from 'shared/api.events';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { GameServerManagerService } from './game-server-manager-service';
import { ApiClient } from 'backend/api/api-client';
import { sdkGamePipeName } from 'backend/constants';
import { Task, TaskReporterService } from 'backend/task/task-reporter-service';
import { NotificationService } from 'backend/notification/notification-service';
import { ShellCommand } from 'backend/process/ShellCommand';
import { GameService } from 'backend/game/game-service';
import { Deferred } from 'backend/deferred';

export interface ServerStartRequest {
  fxserverCwd: string,
  updateChannel: ServerUpdateChannel,
  licenseKey?: string,
  steamWebApiKey?: string,
  tebexSecret?: string,
}

@injectable()
export class GameServerService implements AppContribution, ApiContribution {
  getId() {
    return 'GameServerService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(TaskReporterService)
  protected readonly taskReporterService: TaskReporterService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(GameService)
  protected readonly gameService: GameService;

  @inject(GameServerManagerService)
  protected readonly gameServerManagerService: GameServerManagerService;

  protected state: ServerStates = ServerStates.down;

  protected sdkGameIPCServer: net.Server | null = null;
  protected sdkGameIPCSocket: net.Socket | null = null;

  protected serverCmd: ShellCommand | null = null;
  protected server: cp.ChildProcess | null = null;

  protected startTask: Task | null = null;
  protected stopTask: Task | null = null;

  protected serverLock: Deferred<void> = new Deferred();
  protected serverLocked: boolean = false;

  boot() {
    this.serverLock.resolve();
    process.on('exit', () => this.serverCmd?.stop());
  }

  getState(): ServerStates {
    return this.state;
  }

  @handlesClientEvent(serverApi.ackState)
  ackState() {
    this.apiClient.emit(serverApi.state, this.state);
  }

  stop() {
    if (this.serverCmd) {
      this.stopTask = this.taskReporterService.create('Stopping server');
      this.serverCmd.stop();
    }
  }

  lock() {
    if (this.serverLocked) {
      return;
    }

    this.serverLocked = true;
    this.serverLock = new Deferred();
  }

  unlock() {
    if (!this.serverLocked) {
      return;
    }

    this.serverLocked = false;
    this.serverLock.resolve();
  }

  async start(request: ServerStartRequest) {
    const { fxserverCwd, updateChannel, licenseKey, steamWebApiKey } = request;

    await this.serverLock.promise;

    // Check if port is available
    if (!await this.isPortAvailable(30120)) {
      return this.notificationService.error(`Port 30120 is already taken, make sure nothing is using it`);
    }

    this.toState(ServerStates.booting);
    this.logService.log('Starting server', request);
    this.startTask = this.taskReporterService.create('Starting server');

    const blankPath = this.fsService.joinPath(fxserverCwd, 'blank.cfg');
    if (!await this.fsService.statSafe(blankPath)) {
      await fs.promises.writeFile(blankPath, '');
    }

    const fxserverPath = this.gameServerManagerService.getServerBinaryPath(updateChannel);
    this.logService.log('FXServer path', fxserverPath, request);
    const fxserverArgs = [
      '+exec', 'blank.cfg',
      '+endpoint_add_tcp', '127.0.0.1:30120',
      '+endpoint_add_udp', '127.0.0.1:30120',
      '+set', 'sv_fxdkMode', '1',
      '+set', 'sv_hostname', 'FxDK Dev Server',
      '+set', 'onesync', 'on',
      '+set', 'sv_maxclients', '48',
      '+set', 'svgui_disable', '1',
      '+set', 'steam_webApiKey', steamWebApiKey || 'none',
      '+add_ace', 'resource.sdk-game', 'command', 'allow',
      '+ensure', 'sdk-game',
    ];

    if (this.gameService.getBuildNumber()) {
      fxserverArgs.push('+set', 'sv_enforcegamebuild', this.gameService.getBuildNumber().toString());
    }

    if (licenseKey) {
      fxserverArgs.push('+set', 'sv_licenseKey', licenseKey);
      fxserverArgs.push('+set', 'sv_master1', '');
      fxserverArgs.push('+set', 'sv_endpointPrivacy', '1');

      await this.gameServerManagerService.ensureSvAdhesiveEnabled(updateChannel, true);
    } else {
      fxserverArgs.push('+set', 'sv_lan', '1');

      await this.gameServerManagerService.ensureSvAdhesiveEnabled(updateChannel, false);
    }

    const resourcesNames = await this.fsService.readdir(this.fsService.joinPath(fxserverCwd, 'resources'));

    resourcesNames.forEach((resourceName) => {
      fxserverArgs.push('+ensure', resourceName);
    });

    this.logService.log('FXServer args', fxserverArgs);

    this.serverCmd = new ShellCommand(fxserverPath, fxserverArgs, fxserverCwd);

    this.serverCmd.onClose(() => {
      this.apiClient.emit(serverApi.clearOutput);

      this.logService.log('FXServer terminated');

      if (this.sdkGameIPCServer) {
        this.sdkGameIPCServer.close();
        this.sdkGameIPCServer = null;
        this.sdkGameIPCSocket = null;
      }

      this.serverCmd = null;

      this.startTask?.done();
      this.stopTask?.done();
      this.toState(ServerStates.down);
    });

    this.serverCmd.onError((error) => {
      this.logService.log('Server has failed to start', error);
    });

    this.serverCmd.start();

    await this.initSdkGameIPC();
  }

  refreshResources() {
    this.emitSdkGameEvent('refresh');
  }

  @handlesClientEvent(serverApi.restartResource)
  restartResource(resourceName: string) {
    this.logService.log('Restarting resource', resourceName);

    this.emitSdkGameEvent('restart', resourceName);
  }

  @handlesClientEvent(serverApi.stopResource)
  stopResource(resourceName: string) {
    this.logService.log('Stopping resource', resourceName);

    this.emitSdkGameEvent('stop', resourceName);
  }

  @handlesClientEvent(serverApi.startResource)
  startResource(resourceName: string) {
    this.logService.log('Starting resource', resourceName);

    this.emitSdkGameEvent('start', resourceName);
  }

  @handlesClientEvent(serverApi.sendCommand)
  sendCommand(cmd: string) {
    this.serverCmd?.writeStdin(cmd + '\n');
  }

  toState(newState: ServerStates) {
    this.state = newState;
    this.ackState();
  }

  @handlesClientEvent(serverApi.ackResourcesState)
  protected sendAckResourcesState() {
    this.emitSdkGameEvent('state');
  }

  // IPC with sdk-game resources loaded in fxserver
  private async initSdkGameIPC() {
    this.sdkGameIPCServer = net.createServer();

    this.sdkGameIPCServer.on('connection', (socket) => {
      this.sdkGameIPCSocket = socket;

      this.logService.log('IPC connection!');

      const lineStream = byline.createStream();

      lineStream.on('data', (msg) => {
        try {
          const [type, data] = JSON.parse(msg.toString());

          switch (type) {
            case 'state': {
              return this.apiClient.emit(serverApi.resourcesState, data);
            }
            case 'ready': {
              this.startTask?.done();
              return this.toState(ServerStates.up);
            }
            case 'consoleBuffer': {
              return this.apiClient.emit(serverApi.bufferedOutput, data);
            }
            case 'console': {
              if (!data.message.trim()) {
                return;
              }
              this.apiClient.emit(serverApi.structuredOutputMessage, data);
              return;
            }
          }
        } catch (e) {
          this.logService.log(`Error parsing message from sdk-game:`, msg.toString(), e);
        }
      });

      socket.pipe(lineStream);
    });

    this.sdkGameIPCServer.on('close', () => {
      this.sdkGameIPCServer = null;
      this.sdkGameIPCSocket = null;
    });

    this.sdkGameIPCServer.listen(sdkGamePipeName);
  }

  emitSdkGameEvent(eventType: string, data?: any) {
    if (!this.sdkGameIPCSocket) {
      this.logService.log('No sdk-game IPC socket', { eventType, data });
      return;
    }

    this.logService.log('Emitting event to sdk-game', eventType, data);

    const msg = JSON.stringify([eventType, data]) + '\n';

    this.sdkGameIPCSocket.write(msg);
  }

  private isPortAvailable(port: number): Promise<boolean> {
    return new Promise((resolve) => {
      const server = net.createServer();

      server.on('error', () => resolve(false));
      server.on('listening', () => {
        server.close();
        resolve(true);
      });

      server.listen(port, '127.0.0.1');
    });
  }
}
