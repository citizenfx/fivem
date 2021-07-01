import deepmerge from 'deepmerge';
import { inject, injectable, postConstruct } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import {
  WEApplyAdditionChangeRequest,
  WEApplyPatchRequest,
  WECam,
  WECreateAdditionGroupRequest,
  WEDeleteAdditionGroupRequest,
  WEDeleteAdditionRequest,
  WEMap,
  WESetAdditionGroupNameRequest as WESetAdditionGroupLabelRequest,
  WESetAdditionGroupRequest,
  WESetAdditionRequest,
} from "./world-editor-types";
import { DEFAULT_WORLD_EDITOR_MAP, WORLD_EDITOR_MAP_NO_GROUP } from "./world-editor-constants";
import { handlesClientEvent } from "backend/api/api-decorators";
import { worldEditorApi } from "shared/api.events";
import { GameService } from 'backend/game/game-service';
import { Deferred } from 'backend/deferred';
import { ChangeAwareContainer } from 'backend/change-aware-container';
import { FsThrottledWriter } from 'backend/fs/fs-throttled-writer';
import { ApiClient } from 'backend/api/api-client';
import { omit } from 'utils/omit';
import { ProjectAccess } from 'backend/project/project-access';
import { WorldEditorMapUpgrader } from './world-editor-map-upgrader';

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

  @inject(WorldEditorMapUpgrader)
  protected readonly worldEditorMapUpgrader: WorldEditorMapUpgrader;

  private map: ChangeAwareContainer<WEMap>;
  private mapWriter: FsThrottledWriter<WEMap>;

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
  readonly setCam = (cam: WECam) => this.map.apply((map) => {
    map.meta.cam = cam;
  });

  @handlesClientEvent(worldEditorApi.applyPatch)
  readonly applyPatch = (request: WEApplyPatchRequest) => this.map.apply((map) => {
    map.patches[request.mapDataHash] ??= {};
    map.patches[request.mapDataHash][request.entityHash] = request.patch;
  });

  @handlesClientEvent(worldEditorApi.setAddition)
  readonly setAddition = (request: WESetAdditionRequest) => this.map.apply((map) => {
    map.additions[request.id] = request.object;
  });

  @handlesClientEvent(worldEditorApi.applyAdditionChange)
  readonly applyAdditionChange = (request: WEApplyAdditionChangeRequest) => this.map.apply((map) => {
    const addition = map.additions[request.id];
    if (addition) {
      map.additions[request.id] = {
        ...addition,
        ...omit(request, 'id'),
      };
    }
  });

  @handlesClientEvent(worldEditorApi.deleteAddition)
  readonly deleteAddition = (request: WEDeleteAdditionRequest) => this.map.apply((map) => {
    delete map.additions[request.id];
  });

  @handlesClientEvent(worldEditorApi.setAdditionGroup)
  readonly setAdditionGroup = (request: WESetAdditionGroupRequest) => this.map.apply((map) => {
    if (map.additions[request.additionId]) {
      map.additions[request.additionId].grp = request.grp;
    }
  });

  @handlesClientEvent(worldEditorApi.createAdditionGroup)
  readonly createAdditionGroup = (request: WECreateAdditionGroupRequest) => this.map.apply((map) => {
    map.additionGroups[request.grp] = {
      label: request.label,
    };
  });

  @handlesClientEvent(worldEditorApi.deleteAdditionGroup)
  readonly deleteAdditionGroup = (request: WEDeleteAdditionGroupRequest) => this.map.apply((map) => {
    delete map.additionGroups[request.grp];

    for (const [additionId, addition] of Object.entries(map.additions)) {
      if (addition.grp === request.grp) {
        if (request.deleteAdditions) {
          delete map.additions[additionId];
        } else {
          map.additions[additionId].grp = WORLD_EDITOR_MAP_NO_GROUP;
        }
      }
    }
  });

  @handlesClientEvent(worldEditorApi.setAdditionGroupLabel)
  readonly setAdditionGroupName = (request: WESetAdditionGroupLabelRequest) => this.map.apply((map) => {
    if (map.additionGroups[request.grp]) {
      map.additionGroups[request.grp].label = request.label;
    }
  });

  private async loadMap(): Promise<WEMap> {
    const mapContent = await this.fsService.readFileString(this.mapPath);

    if (mapContent.trim() === '') {
      return deepmerge({}, DEFAULT_WORLD_EDITOR_MAP);
    }

    const map = deepmerge(
      { ...DEFAULT_WORLD_EDITOR_MAP },
      JSON.parse(mapContent),
      {
        arrayMerge(target, source) {
          if (target?.length) {
            return target;
          }

          return source;
        },
      },
    );

    const upgradedMap = await this.worldEditorMapUpgrader.upgrade(map);

    if (upgradedMap !== map) {
      this.mapWriter.write(upgradedMap);
    }

    return upgradedMap;
  }
}
