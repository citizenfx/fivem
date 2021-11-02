import { AppContribution } from "backend/app/app.extensions";
import { ConfigService } from "backend/config-service";
import { FsService } from "backend/fs/fs-service";
import { LogService } from "backend/logger/log-service";
import { ProjectEvents } from "fxdk/project/node/project-events";
import { ProjectStateRuntime } from "fxdk/project/node/runtime/project-state-runtime";
import { inject, injectable, postConstruct } from "inversify";
import { concurrently } from "utils/concurrently";
import { ShellBackend } from 'backend/shell-backend';
import { URL } from 'url';
import { NotificationService } from "backend/notification/notification-service";

interface FXCode {
  getRootPath(): string;
  getRootPagePath(): string;
  getRemoteResourcePath(query: Record<string, string>): string;
  handleWebSocket(socket: import('net').Socket, query: Record<string, string>, headers: Record<string, string>): Promise<void>;
}

@injectable()
export class FXCodeService implements AppContribution {
  @inject(ConfigService)
  protected readonly configService: ConfigService;

  @inject(ShellBackend)
  protected readonly shellBackend: ShellBackend;

  @inject(LogService)
  protected readonly logService: LogService;

  @inject(FsService)
  protected readonly fsService: FsService;

  @inject(NotificationService)
  protected readonly notificationService: NotificationService;

  private fxcode!: FXCode;

  get fxcodeModulePath(): string {
    return this.fsService.joinPath(this.configService.sdkRootFXCode, 'out/fxdk.js');
  }

  private deleteStaleExtensionsPromise: Promise<void> | null = null;

  @postConstruct()
  initialize() {
    this.shellBackend.useStatic('/fxcode-static', this.configService.sdkRootFXCode);

    this.shellBackend.useUpgrade('/', async (req, socket) => {
      if (!req.url) {
        throw new Error('No req.url, but why? It should be handled in ShellBackend');
      }

      if (this.fxcode) {
        await this.fxcode.handleWebSocket(req.socket, getUrlQuery(req.url), req.headers as any);
      } else {
        socket.destroy(new Error('FXCode not started it seems, rip'));
      }
    });

    this.shellBackend.expressApp.get('/fxcode-root', (_req, res) => {
      if (this.fxcode) {
        res.sendFile(this.fxcode.getRootPagePath());
      } else {
        res.send('FXCode not started it seems, rip');
      }
    });

    this.shellBackend.expressApp.get('/vscode-remote-resource', (req, res) => {
      if (this.fxcode) {
        res.sendFile(this.fxcode.getRemoteResourcePath(getUrlQuery(req.url)));
      } else {
        res.send('FXCode not started it seems, rip');
      }
    });
  }

  async boot() {
    ProjectEvents.BeforeLoad.addListener(this.ensureFXCodeFilesForProject);

    await this.ensureFXCodeData();

    this.deleteStaleExtensionsPromise = this.deleteStaleExtensions();
  }

  async beforeAppStart() {
    if (this.deleteStaleExtensionsPromise) {
      await this.deleteStaleExtensionsPromise;
    }

    this.logService.log('Starting FXCode backend');

    await this.startFXCode();

    this.logService.log('FXCode backend started');
  }

  private async startFXCode() {
    const options = {
      devMode: 'true',
      fxcodePath: './fxcode/out/fxdk.js',
      logLevel: 'debug',
    };

    if (this.configService.selfHosted) {
      options.devMode = 'false';
      options.logLevel = 'info';
      options.fxcodePath = this.fsService.joinPath(this.configService.sdkRootFXCode, 'out/fxdk.js');
    }

    process.env.LOG_LEVEL = process.env.VSCODE_LOG_LEVEL = options.logLevel;
    process.env.FXCODE_DEV_MODE = options.devMode;

    try {
      this.fxcode = new (await nativeRequire(this.fxcodeModulePath)).FXCode;
    } catch (e) {
      this.logService.log('Failed to start FXCode backend');
      this.logService.error(e);

      this.notificationService.error('Failed to start FXCode, check client log and report to devs, if needed');
    }
  }

  private ensureFXCodeData() {
    return this.fsService.ensureDir(this.configService.fxcodeDataPath);
  }

  private readonly ensureFXCodeFilesForProject = async (state: ProjectStateRuntime) => {
    const fxcodePath = this.fsService.joinPath(state.path, '.vscode');

    await this.fsService.ensureDir(fxcodePath);

    return concurrently(
      this.fsService.ensureFile(this.fsService.joinPath(fxcodePath, 'settings.json'), '{}'),
      this.fsService.ensureFile(this.fsService.joinPath(fxcodePath, 'tasks.json'), '{}'),
      this.fsService.ensureFile(this.fsService.joinPath(fxcodePath, 'launch.json'), '{}'),
    );
  };

  private async deleteStaleExtensions() {
    const extensionsPath = this.fsService.joinPath(this.configService.fxcodeDataPath, 'user-extensions');

    const obsoleteFilePath = this.fsService.joinPath(extensionsPath, '.obsolete');
    if (!(await this.fsService.statSafe(obsoleteFilePath))) {
      return;
    }

    try {
      const obsoleteContent = await this.fsService.readFileJson<Record<string, boolean>>(obsoleteFilePath);

      const promises: Promise<void>[] = Object.entries(obsoleteContent)
        .filter(([, obsolete]) => obsolete)
        .map(([extensionPath]) => this.fsService.rimraf(this.fsService.joinPath(extensionsPath, extensionPath)));

      await Promise.all(promises);
      await this.fsService.unlink(obsoleteFilePath);
    } catch (e) {
      // carry on
    }
  }
}

function getUrlQuery(url: string): Record<string, string> {
  return [...new URL(url, 'http://dummy.host').searchParams.entries()].reduce((acc, [key, value]) => {
    acc[key] = value;

    return acc;
  }, {});
}
