import { ApiClient } from "backend/api/api-client";
import { handlesClientEvent } from "backend/api/api-decorators";
import { ContainerAccess } from "backend/container-access";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { ProjectAccess } from "fxdk/project/node/project-access";
import { inject, injectable } from "inversify";
import { resourceTemplateScaffolders } from "resource-templates/scaffolders-list";
import { ResourceTemplateScaffolderArgs } from "resource-templates/types";
import { ResourceManifest } from "../common/resourceManifest";
import { ResourceApi } from "../common/resource.api";
import { ResourceAssetConfig } from "../common/resource.types";

@injectable()
export class ResourceCreatorService {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(ContainerAccess)
  protected readonly containerAccess: ContainerAccess;

  @inject(ProjectAccess)
  protected readonly projectAccess: ProjectAccess;

  @handlesClientEvent(ResourceApi.Endpoints.create)
  async createAsset(request: ResourceApi.CreateRequest): Promise<boolean> {
    this.logService.log('Creating resource asset', request);

    const resourcePath = this.fsService.joinPath(request.basePath, request.name);

    await this.fsService.mkdirp(resourcePath);

    // tap manifest file so fxdk knows it is a resource
    await this.fsService.writeFile(this.fsService.joinPath(resourcePath, 'fxmanifest.lua'), '');

    this.projectAccess.withInstance((project) => {
      project.setAssetConfig(resourcePath, {
        enabled: true,
        restartOnChange: true,
      } as ResourceAssetConfig);
    });

    const resourceManifest = new ResourceManifest();

    if (request.resourceTemplateId) {
      const scaffoldingArgs: ResourceTemplateScaffolderArgs = {
        manifest: resourceManifest,
        resourceName: request.name,
        resourcePath,
        resourceTemplateId: request.resourceTemplateId,
      };

      await this.scaffold(scaffoldingArgs);
    }

    await this.saveResourceManifest(resourcePath, resourceManifest);

    this.logService.log('Finished creating asset', request);

    return true;
  }

  private async saveResourceManifest(resourcePath: string, resourceManifest: ResourceManifest) {
    const fxmanifestPath = this.fsService.joinPath(resourcePath, 'fxmanifest.lua');
    const fxmanifestContent = resourceManifest.toString();

    await this.fsService.writeFile(fxmanifestPath, fxmanifestContent);
  }

  private async scaffold(args: ResourceTemplateScaffolderArgs) {
    const scaffolderCtor = resourceTemplateScaffolders[args.resourceTemplateId];
    if (!scaffolderCtor) {
      return;
    }

    await this.containerAccess.resolve(scaffolderCtor).scaffold(args);
  }
}
