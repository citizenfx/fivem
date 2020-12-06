import { inject, injectable } from 'inversify';
import { AssetCreateRequest } from 'shared/api.requests';
import { AssetKind } from '../asset-contribution';
import { Project } from 'server/project/project';
import { ApiClient } from 'server/api/api-client';
import { LogService } from 'server/logger/log-service';
import { FsService } from 'server/fs/fs-service';

@injectable()
export class ResourceKind implements AssetKind {
  @inject(ApiClient)
  protected readonly apiClient: ApiClient;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  async create(project: Project,request: AssetCreateRequest): Promise<boolean> {
    this.logService.log('Creating resource asset', request);

    const resourcePath = this.fsService.joinPath(request.assetPath, request.assetName);

    await this.fsService.mkdirp(resourcePath);

    const fxmanifestPath = this.fsService.joinPath(resourcePath, 'fxmanifest.lua');
    const fxmanifestContent = `
fx_version 'cerulean'
games { 'gta5' }
`.trimStart();

    const manifest = project.getManifest();

    manifest.resources[request.assetName] = {
      name: request.assetName,
      enabled: true,
      restartOnChange: false,
    };

    await Promise.all([
      this.fsService.writeFile(fxmanifestPath, fxmanifestContent),
      project.setManifest(manifest),
    ]);

    this.logService.log('Finish creating asset', request);

    if (request.callback) {
      request.callback();
    }

    return true;
  }
}
