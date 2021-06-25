import deepmerge from 'deepmerge';
import { inject, injectable, postConstruct } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import {
  WorldEditorApplyAdditionChangeRequest,
  WorldEditorApplyPatchRequest,
  WorldEditorCam,
  WorldEditorDeleteAdditionRequest,
  WorldEditorMap,
  WorldEditorSetAdditionGroupNameRequest,
  WorldEditorSetAdditionGroupRequest,
  WorldEditorSetAdditionRequest,
} from "./world-editor-types";
import { DEFAULT_WORLD_EDITOR_MAP } from "./world-editor-constants";
import { handlesClientEvent } from "backend/api/api-decorators";
import { worldEditorApi } from "shared/api.events";
import { GameService } from 'backend/game/game-service';
import { Deferred } from 'backend/deferred';
import { ChangeAwareContainer } from 'backend/change-aware-container';
import { FsThrottledWriter } from 'backend/fs/fs-throttled-writer';
import { ApiClient } from 'backend/api/api-client';
import { omit } from 'utils/omit';
import { ProjectAccess } from 'backend/project/project-access';

@injectable()
export class WorldEditor implements ApiContribution {
  getId() {
    return `WorldEditor(${this.mapPath})`;
  }

  eventDisposers: Function[] = [];

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(GameService)
  protected readonly gameService: GameService;

  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  private map: ChangeAwareContainer<WorldEditorMap>;
  private mapWriter: FsThrottledWriter<WorldEditorMap>;

  private mapPath: string = '';
  private mapLoadDeferred = new Deferred();

  @postConstruct()
  init() {
    this.eventDisposers.push(
      this.gameService.onBackendMessage('we:setCam', this.setCam),
      this.gameService.onBackendMessage('we:applyPatch', this.applyPatch),
      this.gameService.onBackendMessage('we:setAddition', this.setAddition),
      this.gameService.onBackendMessage('we:applyAdditionChange', this.applyAdditionChange),

      this.gameService.onBackendMessage('we:accept', async () => {
        await this.mapLoadDeferred.promise;

        this.gameService.emitEvent('we:map', this.map.get());
      }),
    );
  }

  async open(mapPath: string) {
    this.mapPath = mapPath;

    this.mapWriter = this.fsService.createThrottledFileWriter(this.mapPath, {
      time: 500,
      serialize(content) {
        return JSON.stringify(content);
      },
    });

    this.map = new ChangeAwareContainer(await this.loadMap(), (snapshot) => this.mapWriter.write(snapshot));

    this.mapLoadDeferred.resolve();

    this.apiClient.emit(worldEditorApi.mapLoaded, this.map.get());
  }

  async close() {
    this.eventDisposers.forEach((disposer) => disposer());

    await this.mapWriter.flush();

    // Ask asset to build itself
    await this.projectAccess.withInstance((project) => project.getAssets().get(this.mapPath)?.build?.());
  }

  @handlesClientEvent(worldEditorApi.setCam)
  readonly setCam = (cam: WorldEditorCam) => this.map.apply((map) => {
    map.meta.cam = cam;
  });

  @handlesClientEvent(worldEditorApi.applyPatch)
  readonly applyPatch = (request: WorldEditorApplyPatchRequest) => this.map.apply((map) => {
    map.patches[request.mapDataHash] ??= {};
    map.patches[request.mapDataHash][request.entityHash] = request.patch;
  });

  @handlesClientEvent(worldEditorApi.setAddition)
  readonly setAddition = (request: WorldEditorSetAdditionRequest) => this.map.apply((map) => {
    map.additions[request.id] = request.object;
  });

  @handlesClientEvent(worldEditorApi.applyAdditionChange)
  readonly applyAdditionChange = (request: WorldEditorApplyAdditionChangeRequest) => this.map.apply((map) => {
    const addition = map.additions[request.id];
    if (addition) {
      map.additions[request.id] = {
        ...addition,
        ...omit(request, 'id'),
      };
    }
  });

  @handlesClientEvent(worldEditorApi.deleteAddition)
  readonly deleteAddition = (request: WorldEditorDeleteAdditionRequest) => this.map.apply((map) => {
    delete map.additions[request.id];
  });

  @handlesClientEvent(worldEditorApi.setAdditionGroup)
  readonly setAdditionGroup = (request: WorldEditorSetAdditionGroupRequest) => this.map.apply((map) => {
    if (map.additions[request.id]) {
      map.additions[request.id].grp = request.group;
    }
  });

  @handlesClientEvent(worldEditorApi.setAdditionGroupName)
  readonly setAdditionGroupName = (request: WorldEditorSetAdditionGroupNameRequest) => this.map.apply((map) => {
    if (map.additionGroups[request.additionIndex]) {
      map.additionGroups[request.additionIndex] = request.name;
    }
  });

  private async loadMap(): Promise<WorldEditorMap> {
    const mapContent = await this.fsService.readFileString(this.mapPath);

    if (mapContent.trim() === '') {
      return deepmerge({}, DEFAULT_WORLD_EDITOR_MAP);
    }

    return deepmerge(
      JSON.parse(mapContent),
      DEFAULT_WORLD_EDITOR_MAP,
      {
        arrayMerge(target, source) {
          if (target?.length) {
            return target;
          }

          return source;
        },
      },
    );
  }
}
