import { AssetInterface } from "assets/core/asset-interface";
import { ApiClientScoped } from "backend/api/api-client-scoped";
import { ApiContribution } from "backend/api/api-contribution";
import { handlesClientEvent } from "backend/api/api-decorators";
import { ProjectAccess } from "backend/project/project-access";
import { WorldEditorService } from "backend/world-editor/world-editor-service";
import { inject, injectable, postConstruct } from "inversify";
import { FilesystemEntry } from "shared/api.types";
import { assetTypes } from "shared/asset.types";

@injectable()
export class FXWorld implements AssetInterface, ApiContribution {
  public readonly type = assetTypes.fxworld;

  getId() {
    return this.getName();
  }

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @inject(ApiClientScoped)
  protected readonly apiClient: ApiClientScoped;

  @inject(WorldEditorService)
  protected readonly worldEditorService: WorldEditorService;

  @postConstruct()
  initApiClient() {
    this.apiClient.init(this);
  }

  private entry: FilesystemEntry;

  getName() {
    return this.entry.name;
  }

  getPath() {
    return this.entry.path;
  }

  setEntry(entry: FilesystemEntry) {
    this.entry = entry;

    this.apiClient.setScope(this.entry.name);
  }
}
