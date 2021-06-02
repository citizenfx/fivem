import deepmerge from 'deepmerge';
import { inject, injectable, postConstruct } from "inversify";
import { ApiContribution } from "backend/api/api-contribution";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { WorldEditorApplyPatchRequest, WorldEditorCam, WorldEditorMap } from "./world-editor-types";
import { DEFAULT_WORLD_EDITOR_MAP } from "./world-editor-constants";
import { handlesClientEvent } from "backend/api/api-decorators";
import { worldEditorApi } from "shared/api.events";
import { GameService } from 'backend/game/game-service';
import { Deferred } from 'backend/deferred';
import { ChangeAwareContainer } from 'backend/change-aware-container';
import { FsThrottledWriter } from 'backend/fs/fs-throttled-writer';

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

  private map: ChangeAwareContainer<WorldEditorMap>;
  private mapWriter: FsThrottledWriter<WorldEditorMap>;

  private mapPath: string = '';
  private mapLoadDeferred = new Deferred();

  @postConstruct()
  init() {
    this.eventDisposers.push(
      this.gameService.onBackendMessage('we:setCam', this.setCam.bind(this)),
      this.gameService.onBackendMessage('we:applyPatch', this.applyPatch.bind(this)),
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
  }

  async close() {
    this.eventDisposers.forEach((disposer) => disposer());

    await this.mapWriter.flush();
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
