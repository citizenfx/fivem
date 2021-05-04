import { inject, injectable } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { AppContribution } from "backend/app/app-contribution";
import { ServerStates, ServerUpdateStates } from 'shared/api.types';
import { handlesClientEvent } from 'backend/api/api-decorators';
import { serverApi } from 'shared/api.events';
import { FsService } from 'backend/fs/fs-service';
import { LogService } from 'backend/logger/log-service';
import { GameServerManagerService } from './game-server-manager-service';
import { ApiClient } from 'backend/api/api-client';
import { Task, TaskReporterService } from 'backend/task/task-reporter-service';
import { NotificationService } from 'backend/notification/notification-service';
import { GameService } from 'backend/game/game-service';
import { Deferred } from 'backend/deferred';
import { isPortAvailable } from 'backend/net-utils';
import { ContainerAccess } from 'backend/container-access';
import { GameServerMode } from './game-server-interface';
import { GameServerRuntime, ServerResourceDescriptor, ServerStartRequest } from "./game-server-runtime";
import { GameServerFxdkMode } from './game-server-fxdk-mode';
import { GameServerLegacyMode } from "./game-server-legacy-mode";
import { SingleEventEmitter } from "utils/singleEventEmitter";
import { Disposable } from "backend/disposable-container";

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

  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  @inject(GameServerRuntime)
  protected readonly gameServerRuntime: GameServerRuntime;

  protected state = ServerStates.down;

  protected server: GameServerMode | null = null;

  protected startTask: Task | null = null;
  protected stopTask: Task | null = null;

  protected serverLock = new Deferred<void>();
  protected serverLocked = false;

  private readonly serverStopEvent = new SingleEventEmitter<Error | void>();
  onServerStop(cb: (error: Error | void) => void): Disposable {
    return this.serverStopEvent.addListener(cb);
  }

  private disposeServer() {
    if (this.server) {
      this.server.dispose();
      this.server = null;
    }
  }

  boot() {
    this.serverLock.resolve();
    process.on('exit', () => {
      if (this.server) {
        this.server.stop(this.stopTask = this.taskReporterService.create('Stopping server'));
      }
    });
  }

  getState(): ServerStates {
    return this.state;
  }

  isUp() {
    return this.state === ServerStates.up;
  }

  @handlesClientEvent(serverApi.ackState)
  ackState() {
    this.apiClient.emit(serverApi.state, this.state);
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
    if (this.server) {
      return this.notificationService.warning('Failed to start server as it is already running');
    }

    const { fxserverCwd, updateChannel } = request;

    await this.serverLock.promise;

    this.lock();

    await this.gameServerManagerService.getUpdateChannelPromise(request.updateChannel, ServerUpdateStates.ready);

    // Check if port is available
    if (!await isPortAvailable(30120)) {
      this.notificationService.error(`Port 30120 is already taken, make sure nothing is using it`);
      return this.unlock();
    }

    this.toState(ServerStates.booting);
    this.logService.log('Starting server', request);
    this.startTask = this.taskReporterService.create('Starting server');

    const blankPath = this.fsService.joinPath(fxserverCwd, 'blank.cfg');
    if (!await this.fsService.statSafe(blankPath)) {
      await this.fsService.writeFile(blankPath, '');
    }

    const modeImplService: { new(): GameServerMode } = (await this.gameServerManagerService.getServerSupportsFxdkMode(updateChannel))
      ? GameServerFxdkMode
      : GameServerLegacyMode;

    try {
      this.server = this.containerAccess.resolve(modeImplService);

      this.server.onStop((error) => {
        this.disposeServer();

        if (error) {
          this.notificationService.error(`Server error: ${error.toString()}`);
        }
        this.toState(ServerStates.down);

        this.serverStopEvent.emit(error);
      });

      await this.server.start(request, this.startTask);

      this.toState(ServerStates.up);

    } catch (e) {
      this.disposeServer();

      this.toState(ServerStates.down);
      this.notificationService.error(`Failed to start server: ${e.toString()}`);

    } finally {
      this.startTask.done();
      this.stopTask = null;
      this.unlock();
    }
  }

  async stop() {
    if (this.server) {
      this.lock();

      try {
        this.stopTask = this.taskReporterService.create('Stopping server');
        await this.server.stop(this.stopTask);
      } catch (e) {
        this.notificationService.error(`Failed to stop server: ${e.toString()}`);
      } finally {
        this.stopTask.done();
        this.stopTask = null;
        this.unlock();
      }
    }
  }

  toState(newState: ServerStates) {
    this.state = newState;
    this.ackState();
  }

  @handlesClientEvent(serverApi.startResource)
  startResource(resourceName: string) {
    this.gameServerRuntime.startResource(resourceName);
  }

  @handlesClientEvent(serverApi.stopResource)
  stopResource(resourceName: string) {
    this.gameServerRuntime.stopResource(resourceName);
  }

  @handlesClientEvent(serverApi.restartResource)
  restartResource(resourceName: string) {
    this.gameServerRuntime.restartResource(resourceName);
  }

  reloadResource(resourceName: string) {
    this.gameServerRuntime.reloadResource(resourceName);
  }

  setResources(resources: ServerResourceDescriptor[]) {
    this.gameServerRuntime.setResources(resources);
  }

  getResources(): ServerResourceDescriptor[] {
    return this.gameServerRuntime.getResources();
  }

  @handlesClientEvent(serverApi.sendCommand)
  sendCommand(cmd: string) {
    this.gameServerRuntime.sendCommand(cmd);
  }

  @handlesClientEvent(serverApi.ackResourcesState)
  requestResourcesState() {
    this.gameServerRuntime.requestResourcesState();
  }
}
