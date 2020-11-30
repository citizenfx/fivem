import * as path from 'path';
import * as fs from 'fs';
import * as cp from 'child_process';
import * as net from 'net';
import * as byline from 'byline';
import * as mkdirp from 'mkdirp';
import * as paths from '../paths';
import * as rimrafSync from 'rimraf'
import { promisify } from 'util';
import { ApiClient, Feature, RelinkResourcesRequest, ServerRefreshResourcesRequest as ServerSetEnabledResourcesRequest, ServerStartRequest, ServerStates } from 'shared/api.types';
import { serverApi } from 'shared/api.events';
import { sdkGamePipeName } from './constants';
import { SystemEvent, systemEvents } from './systemEvents';
import { ServerManagerApi } from './ServerManagerApi';
import { createLock } from '../../shared/utils';
import { doesPathExist } from './ExplorerApi';
import { FeaturesApi } from './FeaturesApi';

const rimraf = promisify(rimrafSync);


function getProjectServerPath(projectPath: string): string {
  return path.join(projectPath, '.fxdk/fxserver');
}

export class ServerApi {
  state: ServerStates = ServerStates.down;

  ipcServer: net.Server | null = null;
  ipcSocket: net.Socket | null = null;

  server: cp.ChildProcess | null = null;
  currentEnabledResourcesPaths = new Set<string>();

  serverLock = createLock();

  constructor(
    private readonly client: ApiClient,
    private readonly serverManager: ServerManagerApi,
    private readonly features: FeaturesApi,
  ) {
    systemEvents.on(SystemEvent.refreshResources, () => this.handleRefreshResources());
    systemEvents.on(SystemEvent.relinkResources, (request: RelinkResourcesRequest) => this.handleRelinkResources(request));
    systemEvents.on(SystemEvent.restartResource, (resourceName: string) => this.handleResourceRestart(resourceName));
    systemEvents.on(SystemEvent.forceStopServer, () => this.stop());

    process.on('exit', () => {
      if (this.server) {
        this.server.kill('SIGKILL');
      }
    });

    this.client.on(serverApi.ackState, () => this.ackState());
    this.client.on(serverApi.start, (request: ServerStartRequest) => this.start(request));
    this.client.on(serverApi.stop, () => this.stop());
    this.client.on(serverApi.sendCommand, (cmd: string) => this.sendCommand(cmd));
    this.client.on(serverApi.restartResource, (resourceName: string) => this.handleResourceRestart(resourceName));
    this.client.on(serverApi.setEnabledResources, (request: ServerSetEnabledResourcesRequest) => this.setEnabledResources(request));
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

    const blankPath = path.join(fxserverCwd, 'blank.cfg');
    if (await doesPathExist(blankPath)) {
      await fs.promises.writeFile(blankPath, '');
    }

    await mkdirp(fxserverCwd);
    this.client.log('Ensured FXServer cwd exist');

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

    await this.setupIpc();

    server.stdout.on('data', (data) => {
      this.client.emit(serverApi.output, data.toString('utf8'));
    });

    server.on('exit', () => {
      if (this.ipcServer) {
        this.ipcServer.close();
      }

      this.server = null;

      this.toState(ServerStates.down);
    });

    server.unref();

    this.server = server;
    this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
  }

  async setEnabledResources(request: ServerSetEnabledResourcesRequest) {
    const { projectPath, enabledResourcesPaths } = request;

    const fxserverCwd = getProjectServerPath(projectPath);

    await mkdirp(fxserverCwd);
    await this.linkResources(fxserverCwd, enabledResourcesPaths);

    this.reconcileEnabledResources(enabledResourcesPaths);

    this.sendIpcEvent('refresh');
  }

  handleRefreshResources() {
    this.sendIpcEvent('refresh');
  }

  async handleRelinkResources(request: RelinkResourcesRequest) {
    if (this.state !== ServerStates.up) {
      return;
    }

    await this.serverLock.withLock(async () => {
      const { projectPath, enabledResourcesPaths } = request;

      await this.linkResources(getProjectServerPath(projectPath), enabledResourcesPaths);

      this.reconcileEnabledResources(enabledResourcesPaths);
    });
  }

  handleResourceRestart(resourceName: string) {
    this.client.log('Restarting resource', resourceName);

    this.sendIpcEvent('restart', resourceName);
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

      this.sendIpcEvent('refresh');
    });
  }

  private sendIpcEvent(eventType: string, data?: any) {
    if (!this.ipcSocket) {
      this.client.log('No ipcSocket', eventType, data);
      return;
    }

    this.client.log('Sending ipcEvent', eventType, data);

    const msg = JSON.stringify([eventType, data]) + '\n';

    this.ipcSocket.write(msg);
  }

  // IPC channel to communicate with sdk-game resources loaded in fxserver
  private async setupIpc() {
    let disposableHandlers: (() => void)[] = [];

    this.ipcServer = net.createServer();

    this.ipcServer.on('connection', (socket) => {
      this.ipcSocket = socket;

      this.client.log('IPC connection!');

      disposableHandlers.push(
        this.client.on(serverApi.ackResourcesState, () => {
          this.sendIpcEvent('state');
        }),
        this.client.on(serverApi.restartResource, (resourceName) => {
          this.sendIpcEvent('restart', resourceName);
        }),
        this.client.on(serverApi.stopResource, (resourceName) => {
          this.sendIpcEvent('stop', resourceName);
        }),
        this.client.on(serverApi.startResource, (resourceName) => {
          this.sendIpcEvent('start', resourceName);
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

    this.ipcServer.on('close', () => {
      this.ipcServer = null;
      this.ipcSocket = null;

      // Copy to clear original immediately
      const disposeHandlersCopy = disposableHandlers;
      disposableHandlers = [];

      disposeHandlersCopy.map((disposeHandler) => disposeHandler());
    });

    this.ipcServer.listen(sdkGamePipeName);
  }

  private reconcileEnabledResources(enabledResourcesPaths: string[]) {
    if (this.state !== ServerStates.up) {
      return;
    }

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
        return this.sendIpcEvent('start', resourceName);
      }

      if (state === ResourceReconcilationState.stop) {
        return this.sendIpcEvent('stop', resourceName);
      }
    });

    this.currentEnabledResourcesPaths = new Set(enabledResourcesPaths);
  }
}


enum ResourceReconcilationState {
  start = 1,
  stop,
  idle,
}
