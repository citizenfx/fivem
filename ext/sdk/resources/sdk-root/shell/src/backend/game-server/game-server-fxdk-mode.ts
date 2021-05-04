import { ApiClient } from 'backend/api/api-client';
import { ConfigService } from 'backend/config-service';
import { Deferred } from 'backend/deferred';
import { DisposableContainer, disposableFromFunction } from 'backend/disposable-container';
import { FsService } from 'backend/fs/fs-service';
import { GameService } from 'backend/game/game-service';
import { LogService } from 'backend/logger/log-service';
import { ShellCommand } from 'backend/process/ShellCommand';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { Task } from 'backend/task/task-reporter-service';
import { Ticker } from 'backend/ticker';
import { inject, injectable } from 'inversify';
import { serverApi } from 'shared/api.events';
import { GameServerMode } from "./game-server-interface";
import { GameServerIPC } from './game-server-ipc';
import { GameServerManagerService } from './game-server-manager-service';
import { GameServerResourcesReconciler } from './game-server-resources-reconciler';
import { GameServerRuntime, ServerResourceDescriptor, ServerStartRequest } from './game-server-runtime';
import { getResourceNames, getResourceUrl, getResourceUrls } from './game-server-utils';

@injectable()
export class GameServerFxdkMode implements GameServerMode {
  @inject(GameServerManagerService)
  protected readonly gameServerManagerService: GameServerManagerService;

  @inject(GameService)
  protected readonly gameService: GameService;

  @inject(GameServerRuntime)
  protected readonly gameServerRuntime: GameServerRuntime;

  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  private disposableContainer = new DisposableContainer();

  private shellCommand: ShellCommand | null = null;
  private ipc: GameServerIPC = new GameServerIPC();
  private pipeAppendix: string = '';
  private task: Task;

  private reconciler = new GameServerResourcesReconciler();

  private resourcesStateTicker = new Ticker();
  private resourcesState: Record<string, boolean> = Object.create(null);

  private readonly onCloseEvent = new SingleEventEmitter<Error | void>();
  onStop(cb: (error?: Error) => void) {
    return disposableFromFunction(this.onCloseEvent.addListener(cb));
  }

  private readonly startedDeferred = new Deferred<void>();
  private readonly readyDeferred = new Deferred<void>();

  async start(request: ServerStartRequest, task: Task) {
    this.task = task;

    this.apiClient.emit(serverApi.clearOutput);

    this.setupGameServerServiceEvents();

    this.setupIpcEvents();
    task.setText('IPC events set up');

    await this.startIpc();
    task.setText('IPC started');

    await this.startServer(request);
    task.setText('Server started');

    await this.ipc.connection.promise;
    task.setText('IPC connection');

    await this.serverInitSequence(request);
    task.setText('Server initied');

    await this.readyDeferred.promise;
    task.setText('Server ready');
  }

  async stop(task: Task) {
    if (this.shellCommand) {
      await this.shellCommand.stop();
    }
  }

  private disposed = false;
  async dispose() {
    if (this.disposed) {
      return;
    }

    this.disposed = true;
    this.disposableContainer.dispose();
    this.ipc.dispose();
  }

  async setResources(resources: ServerResourceDescriptor[]) {
    const reconciled = this.reconciler.setResources(resources);

    if (reconciled.unload.length) {
      await this.unloadResources(reconciled.unload);
    }

    if (reconciled.load.length) {
      await this.loadResources(reconciled.load);
      await this.startResources(getResourceNames(reconciled.load));
    }
  }

  async startResource(resourceName: string) {
    try {
      const started = await this.ipc.rpc('start', resourceName);
      this.task.setText(`Started resource ${resourceName}`);

      if (!started) {
        throw new Error(`Failed to start ${resourceName}`);
      }
    } catch (e) {
      throw new Error(`Failed to start ${resourceName} as it is not loaded`);
    }
  }
  async startResources(resourceNames: string[]) {
    const startResults: Record<string, boolean | string> = await this.ipc.rpc('start', resourceNames);

    const failedResourceNames: string[] = [];

    for (const [resourceName, result] of Object.entries(startResults)) {
      if (typeof result === 'string' || !result) {
        failedResourceNames.push(`${resourceName}(result: ${result})`);
      }
    }

    if (failedResourceNames.length) {
      throw new Error(`Failed to start resources: ${failedResourceNames.join(', ')}`);
    }
  }

  async stopResource(resourceName: string) {
    try {
      const stopped = await this.ipc.rpc('stop', resourceName);

      if (!stopped) {
        throw new Error(`Failed to stop ${resourceName}`);
      }
    } catch (e) {
      throw new Error(`Failed to stop ${resourceName} as it is not loaded`);
    }
  }
  async stopResources(resourceNames: string[]) {
    const stopResults: Record<string, boolean | string> = await this.ipc.rpc('stop', resourceNames);

    const failedResourceNames: string[] = [];

    for (const [resourceName, result] of Object.entries(stopResults)) {
      if (typeof result === 'string' || !result) {
        failedResourceNames.push(`${resourceName}(result: ${result})`);
      }
    }

    if (failedResourceNames.length) {
      throw new Error(`Failed to stop resources: ${failedResourceNames.join(', ')}`);
    }
  }

  async restartResource(resourceName: string) {
    try {
      const started = await this.ipc.rpc('restart', resourceName);

      if (!started) {
        throw new Error(`Failed to restart ${resourceName}`);
      }
    } catch (e) {
      throw new Error(`Failed to restart ${resourceName} as it is not loaded`);
    }
  }
  async restartResources(resourceNames: string[]) {
    const restartResults: Record<string, boolean | string> = await this.ipc.rpc('restart', resourceNames);

    const failedResourceNames: string[] = [];

    for (const [resourceName, result] of Object.entries(restartResults)) {
      if (typeof result === 'string' || !result) {
        failedResourceNames.push(`${resourceName}(result: ${result})`);
      }
    }

    if (failedResourceNames.length) {
      throw new Error(`Failed to restart resources: ${failedResourceNames.join(', ')}`);
    }
  }

  acceptCommand(cmd: string) {
    this.ipc.event('cmd', cmd);
  }

  ackResourcesState() {
    this.resourcesStateTicker.whenTickEnds(() => this.apiClient.emit(serverApi.resourcesState, this.resourcesState));
  }

  private loadResources(resources: ServerResourceDescriptor[]): Promise<void> {
    return this.loadResourcesByUrls(getResourceUrls(resources));
  }
  private async loadResourcesByUrls(resourceUrls: string[]): Promise<void> {
    const loadResults = await this.ipc.rpc('load', resourceUrls);

    const failedResources: string[] = [];

    resourceUrls.forEach((resourceUrl) => {
      if (!loadResults[resourceUrl]) {
        failedResources.push(resourceUrl);
      }
    });

    if (failedResources.length) {
      throw new Error(`Failed to load resources: ${failedResources.join(', ')}`);
    }
  }

  private unloadResources(resources: ServerResourceDescriptor[]): Promise<Record<string, boolean>> {
    return this.unloadResourcesByNames(getResourceNames(resources));
  }
  private unloadResourcesByNames(resourceNames: string[]): Promise<Record<string, boolean>>{
    return this.ipc.rpc('unload', resourceNames);
  }

  private reloadResource(resourceName: string): Promise<void> {
    return this.ipc.rpc('reload', [resourceName]);
  }

  private setupGameServerServiceEvents() {
    this.disposableContainer.add(
      this.gameServerRuntime.onSendCommand((cmd: string) => this.acceptCommand(cmd)),
      this.gameServerRuntime.onSetResources((resources: ServerResourceDescriptor[]) => this.setResources(resources)),
      this.gameServerRuntime.onStartResource((resourceName: string) => this.startResource(resourceName)),
      this.gameServerRuntime.onStopResource((resourceName: string) => this.stopResource(resourceName)),
      this.gameServerRuntime.onRestartResource((resourceName: string) => this.restartResource(resourceName)),
      this.gameServerRuntime.onReloadResource((resourceName: string) => this.reloadResource(resourceName)),
      this.gameServerRuntime.onRequestResourcesState(() => this.ackResourcesState()),
    );
  }

  private async startServer({ updateChannel, fxserverCwd }: ServerStartRequest) {
    // Clean up resources folder after legacy mode
    await this.fsService.rimraf(this.fsService.joinPath(fxserverCwd, 'resources'));

    this.shellCommand = new ShellCommand(
      this.gameServerManagerService.getServerBinaryPath(updateChannel),
      ['-fxdk', this.pipeAppendix],
      fxserverCwd,
    );

    const closeErrorHandler = (error?: Error) => {
      this.ipc.dispose();

      this.onCloseEvent.emit(error);
    };

    this.shellCommand.onClose(() => closeErrorHandler());
    this.shellCommand.onError(closeErrorHandler);

    return this.shellCommand.start();
  }

  private setupIpcEvents() {
    this.ipc.onEvent('started', () => this.startedDeferred.resolve());
    this.ipc.onEvent('ready', () => this.readyDeferred.resolve());

    this.ipc.onEvent('stdout', ([channel, message]: [string, string]) => {
      if (message.trim()) {
        this.apiClient.emit(serverApi.structuredOutputMessage, { channel, message });
      }
    });

    this.ipc.onEvent('resource-start', (resourceName: string) => {
      this.resourcesState[resourceName] = true;
      this.ackResourcesState();
    });

    this.ipc.onEvent('resource-stop', (resourceName: string) => {
      delete this.resourcesState[resourceName];
      this.ackResourcesState();
    });
  }

  private async startIpc() {
    this.pipeAppendix = await this.ipc.start('test');
  }

  private async serverInitSequence({ fxserverCwd, steamWebApiKey, tebexSecret, cmdList }: ServerStartRequest) {
    if (Array.isArray(cmdList)) {
      for (const line of cmdList) {
        this.ipc.event('cmd', line.trim());
      }
    } else {
      const blankContent = (await this.fsService.readFile(this.fsService.joinPath(fxserverCwd, 'blank.cfg'))).toString().trim();
      if (blankContent) {
        const lines = blankContent.split('\n');

        for (const line of lines) {
          this.ipc.event('cmd', line.trim());
        }
      }
    }

    if (this.gameService.getBuildNumber()) {
      this.ipc.event('cmd', `set sv_enforcegamebuild ${this.gameService.getBuildNumber()}`);
      this.task.setText('Enforced game build');
    }

    if (tebexSecret) {
      this.ipc.event('cmd', `set sv_tebexSecret ${tebexSecret}`);
    }

    this.ipc.event('cmd', `set steam_webApiKey ${steamWebApiKey || 'none'}`);
    this.task.setText('Steam webapi key set');

    this.ipc.event('initDone');
    this.task.setText('Init done');

    await this.startedDeferred.promise;
    this.task.setText('Started');

    const resources = {
      names: [
        'sdk-game',
      ],
      urls: [
        getResourceUrl({
          name: 'sdk-game',
          path: this.configService.sdkGame,
        }),
      ],
    };

    const resourceDescriptors = this.gameServerRuntime.getResources();

    this.reconciler.setInitialResources(resourceDescriptors);

    resourceDescriptors.forEach((descriptor) => {
      resources.names.push(descriptor.name);
      resources.urls.push(getResourceUrl(descriptor));
    });

    await this.loadResourcesByUrls(resources.urls);
    this.task.setText('Loaded resources');

    await this.startResources(resources.names);
    this.task.setText('Started resources');
  }
}
