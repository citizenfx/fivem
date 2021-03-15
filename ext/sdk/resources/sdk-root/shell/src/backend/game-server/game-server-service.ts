import fs from 'fs';
import cp from 'child_process';
import net from 'net';
import byline from 'byline';
import { inject, injectable } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { AppContribution } from "backend/app/app-contribution";
import { Feature, ServerStates } from 'shared/api.types';
import { handlesClientEvent } from 'backend/api/api-decorators';
import { serverApi } from 'shared/api.events';
import { ServerStartRequest, SetEnabledResourcesRequest } from 'shared/api.requests';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { GameServerManagerService } from './game-server-manager-service';
import { FeaturesService } from 'backend/features/features-service';
import { ConfigService } from 'backend/config-service';
import { ApiClient } from 'backend/api/api-client';
import { sdkGamePipeName } from 'backend/constants';
import { Sequencer } from 'backend/execution-utils/sequencer';
import { Task, TaskReporterService } from 'backend/task/task-reporter-service';
import { NotificationService } from 'backend/notification/notification-service';
import { ShellCommand } from 'backend/process/ShellCommand';
import { GameService } from 'backend/game/game-service';
import { debounce } from 'shared/utils';

enum ResourceReconcilationState {
  start = 1,
  stop,
  idle,
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

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(FeaturesService)
  protected readonly featuresService: FeaturesService;

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
  protected currentEnabledResourcesPaths = new Set<string>();

  protected startTask: Task | null = null;
  protected stopTask: Task | null = null;

  protected serverOpsSequencer = new Sequencer();

  boot() {
    process.on('exit', () => this.serverCmd?.stop());
  }

  getProjectServerPath(projectPath: string): string {
    return this.fsService.joinPath(projectPath, '.fxdk/fxserver');
  }

  getBlankConfigPath(projectPath: string): string {
    return this.fsService.joinPath(this.getProjectServerPath(projectPath), 'blank.cfg');
  }

  @handlesClientEvent(serverApi.ackState)
  ackState() {
    this.apiClient.emit(serverApi.state, this.state);
  }

  @handlesClientEvent(serverApi.stop)
  stop() {
    if (this.serverCmd) {
      this.stopTask = this.taskReporterService.create('Stopping server');
      this.serverCmd.stop();
    }
  }

  @handlesClientEvent(serverApi.start)
  async start(request: ServerStartRequest) {
    const { projectPath, updateChannel, licenseKey, steamWebApiKey } = request;

    // Check if port is available
    if (!await this.isPortAvailable(30120)) {
      return this.notificationService.error(`Port 30120 is already taken, make sure nothing is using it`);
    }

    this.toState(ServerStates.booting);
    this.logService.log('Starting server', request);
    this.startTask = this.taskReporterService.create('Starting server');

    const fxserverCwd = this.getProjectServerPath(projectPath);
    this.logService.log('FXServer cwd', fxserverCwd);

    await this.fsService.mkdirp(fxserverCwd);
    this.logService.log('Ensured FXServer cwd exist');

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

    this.currentEnabledResourcesPaths.forEach((resourcePath) => {
      fxserverArgs.push('+ensure', this.fsService.basename(resourcePath));
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

  @handlesClientEvent(serverApi.setEnabledResources)
  setEnabledResources = debounce(async (request: SetEnabledResourcesRequest) => {
    this.logService.log('Setting enabled resources', request);

    const { projectPath, enabledResourcesPaths } = request;

    const fxserverCwd = this.getProjectServerPath(projectPath);

    if (!await this.fsService.statSafe(fxserverCwd)) {
      await this.fsService.mkdirp(fxserverCwd);
    }

    await this.linkResources(fxserverCwd, enabledResourcesPaths);

    this.reconcileEnabledResourcesAndRefresh(enabledResourcesPaths);
  }, 5);

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

  private async linkResource(source: string, dest: string) {
    const windowsDevModeEnabled = await this.featuresService.get(Feature.windowsDevModeEnabled);

    const linkType = windowsDevModeEnabled
      ? 'dir'
      : 'junction';

    return fs.promises.symlink(source, dest, linkType);
  }

  private async linkResources(fxserverCwd: string, resourcesPaths: string[]) {
    this.logService.log('Linking resources', resourcesPaths);
    await this.serverOpsSequencer.executeBlocking(async () => {
      const resourcesDirectoryPath = this.fsService.joinPath(fxserverCwd, 'resources');

      await this.fsService.rimraf(resourcesDirectoryPath);
      await this.fsService.mkdirp(resourcesDirectoryPath);

      const links = resourcesPaths.map((resourcePath) => ({
        source: resourcePath,
        dest: this.fsService.joinPath(resourcesDirectoryPath, this.fsService.basename(resourcePath)),
      }));

      links.unshift({
        source: this.configService.sdkGame,
        dest: this.fsService.joinPath(resourcesDirectoryPath, 'sdk-game'),
      });

      await Promise.all(
        links.map(async ({ source, dest }) => {
          if (await this.fsService.statSafe(dest)) {
            const destRealpath = await fs.promises.realpath(dest);

            if (destRealpath !== source) {
              await fs.promises.unlink(dest);
            }
          }

          try {
            await this.linkResource(source, dest);
          } catch (e) {
            this.logService.log('Failed to link resource', e.toString());
          }
        }),
      );
    });
    this.logService.log('Linked resources', resourcesPaths);
  }

  /**
   * Starts newly enabled resources,
   * Stops disabled resources
   *
   * Asks server to refresh it's state
   */
  private reconcileEnabledResourcesAndRefresh(enabledResourcesPaths: string[]) {
    if (this.state !== ServerStates.up) {
      this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
      return;
    }

    this.logService.log('Reconciling enabled resources', { enabledResourcesPaths });

    this.emitSdkGameEvent('refresh');

    const resourcesStates = {};

    enabledResourcesPaths.forEach((resourcePath) => {
      const resourceName = this.fsService.basename(resourcePath);

      resourcesStates[resourceName] = this.currentEnabledResourcesPaths.has(resourcePath)
        ? ResourceReconcilationState.idle
        : ResourceReconcilationState.start;
    });

    this.currentEnabledResourcesPaths.forEach((resourcePath) => {
      const resourceName = this.fsService.basename(resourcePath);

      if (!resourcesStates[resourceName]) {
        resourcesStates[resourceName] = ResourceReconcilationState.stop;
      }
    });

    Object.entries(resourcesStates).forEach(([resourceName, state]) => {
      if (state === ResourceReconcilationState.start) {
        return this.emitSdkGameEvent('start', resourceName);
      }

      if (state === ResourceReconcilationState.stop) {
        return this.emitSdkGameEvent('stop', resourceName);
      }
    });

    this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
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

  private emitSdkGameEvent(eventType: string, data?: any) {
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
