import deepmerge from 'deepmerge';
import { inject, injectable, postConstruct } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import {
  WEApplyAdditionChangeRequest,
  WEApplyPatchChangeRequest,
  WECreatePatchRequest,
  WECam,
  WECreateAdditionGroupRequest,
  WECreateAdditionRequest,
  WEDeleteAdditionGroupRequest,
  WEDeleteAdditionRequest,
  WEDeletePatchRequest,
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
import { WEApi, WEApiMethod, WEApiMethodRequest } from './world-editor-game-api';

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
    this.onWEApi(WEApi.SetCam, this.setCam);

    this.onWEApi(WEApi.Accept, async () => {
      await this.mapLoadDeferred.promise;

      this.invokeWEApi(WEApi.Map, this.map.get());
    });

    this.onWEApi(WEApi.PatchCreate, this.createPatch);
    this.onWEApi(WEApi.PatchApplyChange, this.applyPatchChange);

    this.onWEApi(WEApi.AdditionSet, this.setAddition);
    this.onWEApi(WEApi.AdditionApplyChange, this.applyAdditionChange);
  }

  private onWEApi<Method extends WEApiMethod>(method: Method, cb: (request: WEApiMethodRequest<Method>) => void) {
    this.eventDisposers.push(
      this.gameService.onBackendMessage(method, cb),
    );
  }

  private invokeWEApi<Method extends WEApiMethod>(method: Method, request: WEApiMethodRequest<Method>) {
    this.gameService.emitEvent(method, request);
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

  @handlesClientEvent(worldEditorApi.createPatch)
  readonly createPatch = (request: WECreatePatchRequest) => this.map.apply((map) => {
    map.patches[request.mapDataHash] ??= {};
    map.patches[request.mapDataHash][request.entityHash] = request.patch;
  });

  @handlesClientEvent(worldEditorApi.applyPatchChange)
  readonly applyPatchChange = (request: WEApplyPatchChangeRequest) => this.map.apply((map) => {
    const patch = map.patches[request.mapdata]?.[request.entity];
    if (!patch) {
      return;
    }

    if (request.mat) {
      patch.mat = request.mat;
    }

    if (request.label) {
      patch.label = request.label;
    }

    if (request.cam) {
      patch.cam = request.cam;
    }
  });

  @handlesClientEvent(worldEditorApi.deletePatch)
  readonly deletePatch = (request: WEDeletePatchRequest) => this.map.apply((map) => {
    if (map.patches[request.mapDataHash]) {
      delete map.patches[request.mapDataHash][request.entityHash];

      if (Object.keys(map.patches[request.mapDataHash]).length === 0) {
        delete map.patches[request.mapDataHash];
      }
    }
  });

  @handlesClientEvent(worldEditorApi.createAddition)
  readonly createAddition = (request: WECreateAdditionRequest) => this.map.apply((map) => {
    map.additions[request.id] = request.addition;
  });

  @handlesClientEvent(worldEditorApi.setAddition)
  readonly setAddition = (request: WESetAdditionRequest) => this.map.apply((map) => {
    map.additions[request.id] = request.addition;
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
          if (source?.length) {
            return source;
          }

          return target;
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
