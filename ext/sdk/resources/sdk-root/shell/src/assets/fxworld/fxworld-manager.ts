import { ContainerAccess } from "backend/container-access";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { AssetManagerContribution } from "backend/project/asset/asset-manager-contribution";
import { DEFAULT_WORLD_EDITOR_MAP } from "backend/world-editor/world-editor-constants";
import { WEMap } from "backend/world-editor/world-editor-types";
import { inject, injectable } from "inversify";
import { APIRQ } from "shared/api.requests";
import { FilesystemEntry } from "shared/api.types";
import { endsWith } from "utils/stringUtils";
import { FXWorld } from "./fxworld";
import { FXWORLD_FILE_EXT } from "./fxworld-types";

@injectable()
export class FXWorldManager implements AssetManagerContribution {
  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  async createAsset(request: APIRQ.AssetCreate): Promise<boolean> {
    this.logService.log('Creating map asset', request);

    const mapFilePath = this.fsService.joinPath(request.assetPath, request.assetName + FXWORLD_FILE_EXT);

    const mapContent: WEMap = DEFAULT_WORLD_EDITOR_MAP;

    await this.fsService.writeFileJson(mapFilePath, mapContent, false);

    if (request.callback) {
      request.callback();
    }

    return true;
  }

  loadAsset(assetEntry: FilesystemEntry): FXWorld | void {
    if (!assetEntry.isFile || !endsWith(assetEntry.path, FXWORLD_FILE_EXT)) {
      return;
    }

    const asset = this.containerAccess.resolve(FXWorld);

    asset.setEntry(assetEntry);

    return asset;
  }
}
