import { ApiClient } from "backend/api/api-client";
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from "backend/api/api-decorators";
import { ServerResourceDescriptor, ServerStartRequest } from "backend/game-server/game-server-runtime";
import { GameServerService } from "backend/game-server/game-server-service";
import { NotificationService } from "backend/notification/notification-service";
import { ProjectAccess } from "backend/project/project-access";
import { inject, injectable, postConstruct } from "inversify";
import { worldEditorApi } from "shared/api.events";
import { serverUpdateChannels } from "shared/api.types";

const ENABLE_WORLD_EDITOR_SERVER_MODE_CMD = `setr sdk_worldEditorMode 1`;
const DISABLE_WORLD_EDITOR_SERVER_MODE_CMD = `setr sdk_worldEditorMode 0`;

@injectable()
export class WorldEditorService implements ApiContribution {
  getId() {
    return 'WorldEditorService';
  }

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @inject(GameServerService)
  protected readonly gameServerService: GameServerService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  private running = false;
  private resourcesToRestore: ServerResourceDescriptor[] | void;

  @postConstruct()
  init() {
    this.gameServerService.onServerStop((error) => {
      if (this.running) {
        this.running = false;

        if (this.resourcesToRestore) {
          this.gameServerService.setResources(this.resourcesToRestore);

          this.resourcesToRestore = undefined;
        }

        this.apiClient.emit(worldEditorApi.stopped);

        if (error) {
          this.notificationService.error(`World-editor server has stopped with error: ${error.toString()}`);
        }
      }
    });
  }

  @handlesClientEvent(worldEditorApi.start)
  async start() {
    if (this.running) {
      return this.notificationService.error('World editor is already running');
    }

    if (!this.projectAccess.hasInstance()) {
      return this.notificationService.error('No project loaded, impossible to start world-editor');
    }

    this.running = true;

    const project = this.projectAccess.getInstance();

    // If server is already running and running latest update channel - we can't reuse that
    if (this.gameServerService.isUp() && project.getUpdateChannel() === serverUpdateChannels.latest) {
      this.resourcesToRestore = this.gameServerService.getResources();

      this.gameServerService.setResources([]);
      this.gameServerService.sendCommand(ENABLE_WORLD_EDITOR_SERVER_MODE_CMD);

      // restart sdk-game so new mode catches in
      this.gameServerService.restartResource('sdk-game');

      return;
    }

    await this.gameServerService.stop();

    const serverStartRequest: ServerStartRequest = {
      fxserverCwd: project.getFxserverCwd(),
      updateChannel: serverUpdateChannels.latest,

      cmdList: [
        ENABLE_WORLD_EDITOR_SERVER_MODE_CMD,
      ],
    };

    // Unload all resources
    this.gameServerService.setResources([]);

    try {
      await this.gameServerService.start(serverStartRequest);
    } catch (e) {
      this.notificationService.error(`Failed to start world-editor server: ${e.toString()}`);
      this.running = false;
    }
  }

  @handlesClientEvent(worldEditorApi.stop)
  async stop() {
    if (this.resourcesToRestore) {
      this.gameServerService.sendCommand(DISABLE_WORLD_EDITOR_SERVER_MODE_CMD);
      this.gameServerService.restartResource('sdk-game');
      this.gameServerService.setResources(this.resourcesToRestore);

      this.resourcesToRestore = undefined;

      return;
    }

    await this.gameServerService.stop();
  }

  isRunning(): boolean {
    return this.running;
  }
}
