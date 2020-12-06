import fs from 'fs';
import cp from 'child_process';
import net from 'net';
import byline from 'byline';
import { inject, injectable } from "inversify";
import { ApiContribution } from "server/api/api-contribution";
import { AppContribution } from "server/app/app-contribution";
import { Feature, ServerStates } from 'shared/api.types';
import { handlesClientEvent } from 'server/api/api-decorators';
import { serverApi } from 'shared/api.events';
import { ServerStartRequest, SetEnabledResourcesRequest } from 'shared/api.requests';
import { FsService } from 'server/fs/fs-service';
import { LogService } from 'server/logger/log-service';
import { GameServerManagerService } from './game-server-manager-service';
import { FeaturesService } from 'server/features/features-service';
import { ConfigService } from 'server/config-service';
import { ApiClient } from 'server/api/api-client';
import { sdkGamePipeName } from 'server/constants';
import { Sequencer } from 'server/execution-utils/sequencer';

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

  @inject(GameServerManagerService)
  protected readonly gameServerManagerService: GameServerManagerService;

  protected state: ServerStates = ServerStates.down;

  protected sdkGameIPCServer: net.Server | null = null;
  protected sdkGameIPCSocket: net.Socket | null = null;

  protected server: cp.ChildProcess | null = null;
  protected currentEnabledResourcesPaths = new Set<string>();

  protected serverOpsSequencer = new Sequencer();

  boot() {
    process.on('exit', () => this.server?.kill('SIGKILL'));
  }

  getProjectServerPath(projectPath: string): string {
    return this.fsService.joinPath(projectPath, '.fxdk/fxserver');
  }

  @handlesClientEvent(serverApi.ackState)
  ackState() {
    this.logService.log('Server state now', ServerStates[this.state]);
    this.apiClient.emit(serverApi.state, this.state);
  }

  @handlesClientEvent(serverApi.stop)
  stop() {
    this.server?.kill('SIGKILL');
  }

  @handlesClientEvent(serverApi.start)
  async start(request: ServerStartRequest) {
    this.logService.log('Starting server', request);

    const { projectPath, enabledResourcesPaths } = request;

    this.logService.log('Ops sequencer state', this.serverOpsSequencer.getStateString(), this.serverOpsSequencer);

    this.apiClient.emit(serverApi.clearOutput);

    this.toState(ServerStates.booting);

    const fxserverCwd = this.getProjectServerPath(projectPath);
    this.logService.log('FXServer cwd', fxserverCwd);

    await this.fsService.mkdirp(fxserverCwd);
    this.logService.log('Ensured FXServer cwd exist');

    const blankPath = this.fsService.joinPath(fxserverCwd, 'blank.cfg');
    if (!await this.fsService.statSafe(blankPath)) {
      await fs.promises.writeFile(blankPath, '');
    }

    await this.linkResources(fxserverCwd, enabledResourcesPaths);
    this.logService.log('Linked resources');

    const fxserverPath = this.gameServerManagerService.getServerBinaryPath(request.updateChannel);
    this.logService.log('FXServer path', fxserverPath, request);
    const fxserverArgs = [
      '+exec', 'blank.cfg',
      '+endpoint_add_tcp', '127.0.0.1:30120',
      '+endpoint_add_udp', '127.0.0.1:30120',
      '+set', 'onesync', 'on',
      '+set', 'sv_maxclients', '64',
      '+set', 'sv_lan', '1',
      '+set', 'steam_webApiKey', 'none',
      '+set', 'svgui_disable', '1',
      '+add_ace', 'resource.sdk-game', 'command', 'allow',
      '+ensure', 'sdk-game',
    ];

    enabledResourcesPaths.forEach((resourcePath) => {
      fxserverArgs.push('+ensure', this.fsService.basename(resourcePath));
    });

    const server = cp.execFile(
      fxserverPath,
      fxserverArgs,
      {
        cwd: fxserverCwd,
        windowsHide: true,
      },
    );

    if (!server || !server.stdout) {
      this.logService.log('Server has failed to start');
      this.toState(ServerStates.down);
      return;
    }

    await this.initSdkGameIPC();

    server.stdout.on('data', (data) => {
      this.apiClient.emit(serverApi.output, data.toString('utf8'));
    });

    server.on('exit', () => {
      this.logService.log('fxserver terminated');

      if (this.sdkGameIPCServer) {
        this.sdkGameIPCServer.close();
      }

      this.server = null;

      this.toState(ServerStates.down);
    });

    server.unref();

    this.server = server;
    this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
  }

  @handlesClientEvent(serverApi.setEnabledResources)
  async setEnabledResources(request: SetEnabledResourcesRequest) {
    this.logService.log('Setting enabled resources', request);

    const { projectPath, enabledResourcesPaths } = request;

    const fxserverCwd = this.getProjectServerPath(projectPath);

    if (!await this.fsService.statSafe(fxserverCwd)) {
      await this.fsService.mkdirp(fxserverCwd);
    }

    await this.linkResources(fxserverCwd, enabledResourcesPaths);

    this.reconcileEnabledResourcesAndRefresh(enabledResourcesPaths);
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
    this.server?.stdin?.write(cmd + '\n');
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
  }

  /**
   * Starts newly enabled resources,
   * Stops disabled resources
   *
   * Asks server to refresh it's state
   */
  private reconcileEnabledResourcesAndRefresh(enabledResourcesPaths: string[]) {
    if (this.state !== ServerStates.up) {
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
              return this.toState(ServerStates.up);
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
}
