import path from 'path';
import fs from 'fs';
import cp from 'child_process';
import net from 'net';
import byline from 'byline';
import mkdirp from 'mkdirp';
import * as paths from '../paths';
import { rimraf } from 'server/rimraf';
import { ApiClient, Feature, ServerStates } from 'shared/api.types';
import { serverApi } from 'shared/api.events';
import { sdkGamePipeName } from './constants';
import { SystemEvent, systemEvents } from './systemEvents';
import { ServerManagerApi } from './ServerManagerApi';
import { createLock } from '../../shared/utils';
import { doesPathExist } from './ExplorerApi';
import { FeaturesApi } from './FeaturesApi';
import { RelinkResourcesRequest, ServerRefreshResourcesRequest as SetEnabledResourcesRequest, ServerStartRequest } from 'shared/api.requests';
import { ApiBase } from './ApiBase';
import { TheiaContext } from 'contexts/TheiaContext';


function getProjectServerPath(projectPath: string): string {
  return path.join(projectPath, '.fxdk/fxserver');
}

export class ServerApi extends ApiBase {
  state: ServerStates = ServerStates.down;

  sdkGameIPCServer: net.Server | null = null;
  sdkGameIPCSocket: net.Socket | null = null;

  server: cp.ChildProcess | null = null;
  currentEnabledResourcesPaths = new Set<string>();

  serverLock = createLock();

  constructor(
    private readonly client: ApiClient,
    private readonly serverManager: ServerManagerApi,
    private readonly features: FeaturesApi,
  ) {
    super();

    systemEvents.on(SystemEvent.refreshResources, this.bind(this.handleRefreshResources));
    systemEvents.on(SystemEvent.relinkResources, this.bind(this.handleRelinkResources));
    systemEvents.on(SystemEvent.restartResource, this.bind(this.handleResourceRestart));
    systemEvents.on(SystemEvent.forceStopServer, this.bind(this.stop));

    process.on('exit', () => {
      if (this.server) {
        this.server.kill('SIGKILL');
      }
    });

    this.client.on(serverApi.ackState, this.bind(this.ackState));
    this.client.on(serverApi.start, this.bind(this.start));
    this.client.on(serverApi.stop, this.bind(this.stop));
    this.client.on(serverApi.sendCommand, this.bind(this.sendCommand));
    this.client.on(serverApi.restartResource, this.bind(this.handleResourceRestart));
    this.client.on(serverApi.setEnabledResources, this.bind(this.setEnabledResources));
  }

  ackState() {
    this.client.emit(serverApi.state, this.state);
  }

  stop() {
    if (this.server) {
      this.server.kill('SIGKILL');
    }
  }

  async start(request: ServerStartRequest) {
    this.client.log('Starting server', request);

    const { projectPath, enabledResourcesPaths } = request;

    await this.serverLock.waitForUnlock();

    this.client.emit(serverApi.clearOutput);

    this.toState(ServerStates.booting);

    const fxserverCwd = getProjectServerPath(projectPath);
    this.client.log('FXServer cwd', fxserverCwd);

    await mkdirp(fxserverCwd);
    this.client.log('Ensured FXServer cwd exist');

    const blankPath = path.join(fxserverCwd, 'blank.cfg');
    if (!await doesPathExist(blankPath)) {
      await fs.promises.writeFile(blankPath, '');
    }

    await this.linkResources(fxserverCwd, enabledResourcesPaths);
    this.client.log('Linked resources');

    const fxserverPath = this.serverManager.getServerBinaryPath(request.updateChannel);
    this.client.log('FXServer path', fxserverPath, request);
    const fxserverArgs = [
      '+exec', 'blank.cfg',
      '+endpoint_add_tcp', '127.0.0.1:30120',
      '+endpoint_add_udp', '127.0.0.1:30120',
      '+set', 'onesync', 'on',
      '+set', 'sv_maxclients', '64',
      '+set', 'sv_lan', '1',
      '+set', 'svgui_disable', '1',
      '+add_ace', 'resource.sdk-game', 'command', 'allow',
      '+ensure', 'sdk-game',
    ];

    enabledResourcesPaths.forEach((resourcePath) => {
      fxserverArgs.push('+ensure', path.basename(resourcePath));
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
      this.client.log('Server has failed to start');
      this.toState(ServerStates.down);
      return;
    }

    await this.initSdkGameIPC();

    server.stdout.on('data', (data) => {
      this.client.emit(serverApi.output, data.toString('utf8'));
    });

    server.on('exit', () => {
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

  async setEnabledResources(request: SetEnabledResourcesRequest) {
    this.client.log('Setting enabled resources', request);

    const { projectPath, enabledResourcesPaths } = request;

    const fxserverCwd = getProjectServerPath(projectPath);

    await mkdirp(fxserverCwd);
    await this.linkResources(fxserverCwd, enabledResourcesPaths);

    this.reconcileEnabledResourcesAndRefresh(enabledResourcesPaths);
  }

  handleRefreshResources() {
    this.emitSdkGameEvent('refresh');
  }

  async handleRelinkResources(request: RelinkResourcesRequest) {
    if (this.state !== ServerStates.up) {
      return;
    }

    this.client.log('Relinking resources', request);

    await this.serverLock.withLock(async () => {
      const { projectPath, enabledResourcesPaths } = request;

      await this.linkResources(getProjectServerPath(projectPath), enabledResourcesPaths);

      this.reconcileEnabledResourcesAndRefresh(enabledResourcesPaths);
    });
  }

  handleResourceRestart(resourceName: string) {
    this.client.log('Restarting resource', resourceName);

    this.emitSdkGameEvent('restart', resourceName);
  }

  sendCommand(cmd: string) {
    this.server?.stdin?.write(cmd + '\n');
  }

  toState(newState: ServerStates) {
    this.state = newState;
    this.ackState();
  }

  private async linkResource(source: string, dest: string) {
    const windowsDevModeEnabled = await this.features.get(Feature.windowsDevModeEnabled);

    const linkType = windowsDevModeEnabled
      ? 'dir'
      : 'junction';

    return fs.promises.symlink(source, dest, linkType);
  }

  private async linkResources(fxserverCwd: string, resourcesPaths: string[]) {
    await this.serverLock.withLock(async () => {
      const resourcesDirectoryPath = path.join(fxserverCwd, 'resources');

      await rimraf(resourcesDirectoryPath);
      await mkdirp(resourcesDirectoryPath);

      const links = resourcesPaths.map((resourcePath) => ({
        source: resourcePath,
        dest: path.join(resourcesDirectoryPath, path.basename(resourcePath)),
      }));

      links.unshift({
        source: paths.sdkGame,
        dest: path.join(resourcesDirectoryPath, 'sdk-game'),
      });

      await Promise.all(
        links.map(async ({ source, dest }) => {
          if (await doesPathExist(dest)) {
            const destRealpath = await fs.promises.realpath(dest);

            if (destRealpath !== source) {
              await fs.promises.unlink(dest);
            }
          }

          try {
            await this.linkResource(source, dest);
          } catch (e) {
            this.client.log('Failed to link resource', e.toString());
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

    this.client.log('Reconciling enabled resources', { enabledResourcesPaths });

    this.emitSdkGameEvent('refresh');

    const resourcesStates = {};

    enabledResourcesPaths.forEach((resourcePath) => {
      const resourceName = path.basename(resourcePath);

      resourcesStates[resourceName] = this.currentEnabledResourcesPaths.has(resourcePath)
        ? ResourceReconcilationState.idle
        : ResourceReconcilationState.start;
    });

    this.currentEnabledResourcesPaths.forEach((resourcePath) => {
      const resourceName = path.basename(resourcePath);

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

  // IPC with sdk-game resources loaded in fxserver
  private async initSdkGameIPC() {
    let disposableHandlers: (() => void)[] = [];

    this.sdkGameIPCServer = net.createServer();

    this.sdkGameIPCServer.on('connection', (socket) => {
      this.sdkGameIPCSocket = socket;

      this.client.log('IPC connection!');

      disposableHandlers.push(
        this.client.on(serverApi.ackResourcesState, () => {
          this.emitSdkGameEvent('state');
        }),
        this.client.on(serverApi.restartResource, (resourceName) => {
          this.emitSdkGameEvent('restart', resourceName);
        }),
        this.client.on(serverApi.stopResource, (resourceName) => {
          this.emitSdkGameEvent('stop', resourceName);
        }),
        this.client.on(serverApi.startResource, (resourceName) => {
          this.emitSdkGameEvent('start', resourceName);
        }),
      );

      const lineStream = byline.createStream();

      lineStream.on('data', (msg) => {
        try {
          const [type, data] = JSON.parse(msg.toString());

          switch (type) {
            case 'state': {
              return this.client.emit(serverApi.resourcesState, data);
            }
            case 'ready': {
              return this.toState(ServerStates.up);
            }
          }
        } catch (e) {
          this.client.log(`Error parsing message from sdk-game:`, msg.toString(), e);
        }
      });

      socket.pipe(lineStream);
    });

    this.sdkGameIPCServer.on('close', () => {
      this.sdkGameIPCServer = null;
      this.sdkGameIPCSocket = null;

      // Copy to clear original immediately
      const disposeHandlersCopy = disposableHandlers;
      disposableHandlers = [];

      disposeHandlersCopy.map((disposeHandler) => disposeHandler());
    });

    this.sdkGameIPCServer.listen(sdkGamePipeName);
  }

  private emitSdkGameEvent(eventType: string, data?: any) {
    if (!this.sdkGameIPCSocket) {
      this.client.log('No sdk-game IPC socket', { eventType, data });
      return;
    }

    this.client.log('Emitting event to sdk-game', eventType, data);

    const msg = JSON.stringify([eventType, data]) + '\n';

    this.sdkGameIPCSocket.write(msg);
  }
}


enum ResourceReconcilationState {
  start = 1,
  stop,
  idle,
}
