import { injectable } from "inversify";
import path from 'path';

if (!process.env.LOCALAPPDATA) {
  console.error('No LOCALAPPDATA env variable');
  process.exit(-1);
}

@injectable()
export class ConfigService {
  readonly shellBackendPort: number;
  readonly theiaBackendPort: number;

  readonly sdkUrl: string;
  readonly selfHosted: boolean;

  readonly realCwd: string = process.cwd();
  readonly citizen: string = path.normalize(GetConvar('citizen_path'));

  readonly sdkResources: string;

  readonly sdkRoot: string;
  readonly sdkGame: string;

  readonly sdkRootShell: string;
  readonly sdkRootShellBuild: string;

  readonly sdkRootTheia: string;
  readonly sdkRootTheiaArchive: string;

  readonly localAppData: string;
  readonly serverArtifacts: string;
  readonly serverContainer: string;
  readonly serverDataPath: string;

  readonly recentProjectsFilePath: string;

  readonly wellKnownPathsPath: string;

  constructor() {
    this.shellBackendPort = 35419;
    this.theiaBackendPort = 35420;

    this.sdkUrl = GetConvar('sdk_url');
    this.selfHosted = this.sdkUrl === 'http://localhost:35419/';

    this.realCwd = process.cwd();
    this.citizen = path.normalize(GetConvar('citizen_path'));

    // __dirname will be like `resources/sdk-root/shell/build_server/index.js`
    this.sdkResources = path.join(__dirname, '../../..');

    this.sdkRoot = path.join(this.sdkResources, 'sdk-root');
    this.sdkGame = path.join(this.sdkResources, 'sdk-game');

    this.sdkRootShell = path.join(this.sdkRoot, 'shell');
    this.sdkRootShellBuild = path.join(this.sdkRootShell, 'build');

    this.sdkRootTheia = path.join(this.sdkRoot, 'personality-theia');
    this.sdkRootTheiaArchive = path.join(this.sdkRoot, 'personality-theia.tar');

    this.localAppData = path.join(process.env.LOCALAPPDATA, 'citizenfx/sdk-storage');
    this.serverArtifacts = path.join(this.localAppData, 'artifacts');
    this.serverContainer = path.join(this.localAppData, 'server');
    this.serverDataPath = path.join(this.localAppData, 'serverData');

    this.recentProjectsFilePath = path.join(this.localAppData, 'recent-projects.json');

    this.wellKnownPathsPath = path.join(this.localAppData, 'well-known-paths.json');
  }
}
