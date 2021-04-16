import fs from 'fs';
import net from 'net';
import byline from 'byline';
import { ApiClient } from 'backend/api/api-client';
import { DisposableContainer, disposableFromFunction } from 'backend/disposable-container';
import { FsService } from 'backend/fs/fs-service';
import { GameService } from 'backend/game/game-service';
import { LogService } from 'backend/logger/log-service';
import { ShellCommand } from 'backend/process/ShellCommand';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { Task } from 'backend/task/task-reporter-service';
import { Ticker } from 'backend/ticker';
import { injectable, inject } from 'inversify';
import { serverApi } from 'shared/api.events';
import { GameServerMode } from './game-server-interface';
import { GameServerManagerService } from './game-server-manager-service';
import { Deferred } from 'backend/deferred';
import { GameServerResourcesReconciler } from './game-server-resources-reconciler';
import { ConfigService } from 'backend/config-service';
import { getResourcePaths } from './game-server-utils';
import { Feature } from 'shared/api.types';
import { FeaturesService } from 'backend/features/features-service';
import { Queue } from 'backend/queue';
import { GameServerRuntime, ServerResourceDescriptor, ServerStartRequest } from './game-server-runtime';
import { fastRandomId } from 'utils/random';

const pipeBaseName = '\\\\.\\pipe\\fxdk_fxserver_sdk_game_';

@injectable()
export class GameServerLegacyMode implements GameServerMode {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(GameService)
  protected readonly gameService: GameService;

  @inject(GameServerRuntime)
  protected readonly gameServerRuntime: GameServerRuntime;

  @inject(GameServerManagerService)
  protected readonly gameServerManagerService: GameServerManagerService;

  @inject(FeaturesService)
  protected readonly featuresService: FeaturesService;

  private fxserverCwd = '';
  private pipeAppendix = '';

  private disposableContainer = new DisposableContainer();

  private serverCmd: ShellCommand | null = null;

  private sdkGameIPCServer: net.Server | null = null;
  private sdkGameIPCSocket: net.Socket | null = null;

  private readyDeferred = new Deferred<void>();

  private resourcesStateTicker = new Ticker();

  async start(request: ServerStartRequest, task: Task) {
    const { fxserverCwd, updateChannel, steamWebApiKey, licenseKey } = request;

    const resources = this.gameServerRuntime.getResources()

    this.fxserverCwd = fxserverCwd;
    this.pipeAppendix = fastRandomId();

    this.reconciler.setInitialResources(resources);

    await this.linkResources(getResourcePaths(resources));
    task.setText('Linked resources');

    this.setupGameServerServiceEvents();
    task.setText('Events set up');

    this.apiClient.emit(serverApi.clearOutput);

    await this.initSdkGameIPC();
    task.setText('IPC initialized');

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
      '+set', 'sv_fxdkPipeAppendix', this.pipeAppendix,
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

    this.disposableContainer.add(() => this.serverCmd = null);

    this.serverCmd.onClose(() => {
      this.logService.log('FXServer terminated');

      this.onStopEvent.emit();
    });

    this.serverCmd.onError((error) => {
      this.logService.log('Server has failed to start', error);
    });

    await this.serverCmd.start();
    task.setText('Server started');

    await this.readyDeferred.promise;
    task.setText('Ready');
  }

  async stop(task: Task) {
    if (this.serverCmd) {
      await this.serverCmd.stop();
    }
  }

  private disposed = false;
  async dispose() {
    if (this.disposed) {
      return;
    }

    this.disposed = true;

    if (this.sdkGameIPCServer) {
      this.sdkGameIPCServer.close();
      this.sdkGameIPCServer = null;
      this.sdkGameIPCSocket = null;
    }

    this.disposableContainer.dispose();
  }

  private onStopEvent = new SingleEventEmitter<Error | void>();
  onStop(cb: (error?: Error) => void) {
    return disposableFromFunction(this.onStopEvent.addListener(cb));
  }

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
              return this.resourcesStateTicker.whenTickEnds(() => this.apiClient.emit(serverApi.resourcesState, data));
            }
            case 'ready': {
              return this.readyDeferred.resolve();
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
          this.logService.error(e, { msg });
        }
      });

      socket.pipe(lineStream);
    });

    const listenDeferred = new Deferred<void>();

    this.sdkGameIPCServer.listen(pipeBaseName + this.pipeAppendix, () => listenDeferred.resolve());

    await listenDeferred.promise;
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

  private setupGameServerServiceEvents() {
    this.disposableContainer.add(
      this.gameServerRuntime.onSendCommand((cmd: string) => this.sendCommand(cmd)),
      this.gameServerRuntime.onSetResources((resources: ServerResourceDescriptor[]) => this.queue.append(resources)),
      this.gameServerRuntime.onStartResource((resourceName: string) => this.startResource(resourceName)),
      this.gameServerRuntime.onStopResource((resourceName: string) => this.stopResource(resourceName)),
      this.gameServerRuntime.onRestartResource((resourceName: string) => this.restartResource(resourceName)),
      this.gameServerRuntime.onReloadResource((resourceName: string) => this.refreshResources()),
      this.gameServerRuntime.onRequestResourcesState(() => this.emitSdkGameEvent('state')),
    );
  }

  private refreshResources() {
    this.emitSdkGameEvent('refresh');
  }

  private restartResource(resourceName: string) {
    this.logService.log('Restarting resource', resourceName);

    this.emitSdkGameEvent('restart', resourceName);
  }

  private stopResource(resourceName: string) {
    this.logService.log('Stopping resource', resourceName);

    this.emitSdkGameEvent('stop', resourceName);
  }

  private startResource(resourceName: string) {
    this.logService.log('Starting resource', resourceName);

    this.emitSdkGameEvent('start', resourceName);
  }

  private sendCommand(cmd: string) {
    this.serverCmd?.writeStdin(cmd + '\n');
  }

  private queue = new Queue<ServerResourceDescriptor[]>((resources) => this.setResources(resources));
  private reconciler = new GameServerResourcesReconciler();
  private async setResources(resources: ServerResourceDescriptor[]) {
    const reconciled = this.reconciler.setResources(resources);

    if (!reconciled.load.length && !reconciled.unload.length) {
      return;
    }

    if (reconciled.unload.length) {
      for (const resource of reconciled.unload) {
        this.stopResource(resource.name);
      }
    }

    await this.linkResources(getResourcePaths(resources));

    this.refreshResources();

    if (reconciled.load.length) {
      for (const resource of reconciled.load) {
        this.startResource(resource.name);
      }
    }
  }

  private async linkResources(resourcesPaths: string[]) {
    const resourcesDirectoryPath = this.fsService.joinPath(this.fxserverCwd, 'resources');

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
  }

  private windowsDevModeEnabled: boolean | void = undefined;
  private async linkResource(source: string, dest: string) {
    if (this.windowsDevModeEnabled === undefined) {
      this.windowsDevModeEnabled = await this.featuresService.get(Feature.windowsDevModeEnabled);
    }

    const linkType = this.windowsDevModeEnabled
      ? 'dir'
      : 'junction';

    return fs.promises.symlink(source, dest, linkType);
  }
}
