import { inject, injectable } from "inversify";
import { AppContribution } from "server/app/app-contribution";
import { ConfigService } from "server/config-service";
import { FsService } from "server/fs/fs-service";

@injectable()
export class TheiaService implements AppContribution {
  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(FsService)
  protected readonly fsService: FsService;

  async beforeAppStart() {
    if (!this.configService.selfHosted) {
      return;
    }

    const theiaBackendPath = this.fsService.joinPath(
      this.configService.sdkRootTheia,
      'backend',
    );

    try {
      process.chdir(this.configService.sdkRootTheia);
      const fakeArgv = [...process.argv, '--plugins=local-dir:plugins'];

      await nativeRequire(theiaBackendPath)(this.configService.theiaBackendPort, 'localhost', fakeArgv);
    } catch (e) {
      console.log('personality-theia has failed to start');
      console.error(e);
    }
  }
}
