import { inject, injectable } from "inversify";
import { FsService } from "backend/fs/fs-service";
import { concurrently } from "utils/concurrently";
import { ResourceTemplateScaffolder, ResourceTemplateScaffolderArgs } from "../types";

@injectable()
export default class LuaScaffolder implements ResourceTemplateScaffolder {
  @inject(FsService)
  protected readonly fsService: FsService;

  async scaffold({ manifest, resourcePath }: ResourceTemplateScaffolderArgs) {
    manifest.clientScripts.push('client.lua');
    manifest.serverScripts.push('server.lua');
    manifest.sharedScripts.push('shared.lua');

    const clientPath = this.fsService.joinPath(resourcePath, 'client.lua');
    const serverPath = this.fsService.joinPath(resourcePath, 'server.lua');
    const sharedPath = this.fsService.joinPath(resourcePath, 'shared.lua');

    await concurrently(
      this.fsService.writeFile(clientPath, ''),
      this.fsService.writeFile(serverPath, ''),
      this.fsService.writeFile(sharedPath, ''),
    );
  }
}
