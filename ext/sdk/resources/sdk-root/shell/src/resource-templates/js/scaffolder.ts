import { inject, injectable } from "inversify";
import { FsService } from "backend/fs/fs-service";
import { concurrently } from "utils/concurrently";
import { ResourceTemplateScaffolder, ResourceTemplateScaffolderArgs } from "../types";

@injectable()
export default class JsScaffolder implements ResourceTemplateScaffolder {
  @inject(FsService)
  protected readonly fsService: FsService;

  async scaffold({ manifest, resourcePath }: ResourceTemplateScaffolderArgs) {
    manifest.clientScripts.push('client.js');
    manifest.serverScripts.push('server.js');
    manifest.sharedScripts.push('shared.js');

    const clientPath = this.fsService.joinPath(resourcePath, 'client.js');
    const serverPath = this.fsService.joinPath(resourcePath, 'server.js');
    const sharedPath = this.fsService.joinPath(resourcePath, 'shared.js');

    await concurrently(
      this.fsService.writeFile(clientPath, ''),
      this.fsService.writeFile(serverPath, ''),
      this.fsService.writeFile(sharedPath, ''),
    );
  }
}
