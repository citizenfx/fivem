import { handlesClientEvent } from "backend/api/api-decorators";
import { ContainerAccess } from "backend/container-access";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { NotificationService } from "backend/notification/notification-service";
import { DEFAULT_WORLD_EDITOR_MAP } from "backend/world-editor/world-editor-constants";
import { WEMap } from "backend/world-editor/world-editor-types";
import { ProjectAccess } from "fxdk/project/node/project-access";
import { inject, injectable } from "inversify";
import { FXWorldAssetConfig, FXWORLD_FILE_EXT } from "../common/fxworld-types";
import { FXWorldApi } from "../common/fxworld.api";

@injectable()
export class FXWorldCreatorService {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @handlesClientEvent(FXWorldApi.Endpoints.create)
  async create(request: FXWorldApi.CreateRequest) {
    await this.projectAccess.useInstance('create map asset', async (project) => {
      this.logService.log('Creating map asset', request);

      const mapFilePath = this.fsService.joinPath(request.basePath, request.name + FXWORLD_FILE_EXT);

      if (await this.fsService.statSafe(mapFilePath)) {
        this.notificationService.error(`Unable to create map ${request.name} as file with the same name already exists`);
        return;
      }

      const mapContent: WEMap = DEFAULT_WORLD_EDITOR_MAP;

      await this.fsService.writeFileJson(mapFilePath, mapContent, false);

      project.setAssetConfig(mapFilePath, {
        enabled: true,
      } as FXWorldAssetConfig);
    });
  }
}
