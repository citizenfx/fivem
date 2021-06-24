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

  readonly cfxLocalAppData: string;

  readonly sdkResources: string;

  readonly sdkRoot: string;
  readonly sdkGame: string;

  readonly sdkRootShell: string;
  readonly sdkRootShellBuild: string;

  readonly theiaConfigPath: string;
  readonly theiaPluginsPath: string;

  readonly sdkRootTheia: string;
  readonly sdkRootTheiaArchive: string;

  readonly sdkStorage: string;
  readonly serverArtifacts: string;
  readonly serverContainer: string;
  readonly serverDataPath: string;

  readonly systemResourcesRoot: string;
  readonly systemResourcesPath: string;

  readonly nativesDocluaPath: string;
  readonly recentProjectsFilePath: string;
  readonly featuresFilePath: string;

  readonly wellKnownPathsPath: string;

  constructor() {
    this.shellBackendPort = 35419;
    this.theiaBackendPort = 35420;

    this.sdkUrl = GetConvar('sdk_url');
    this.selfHosted = this.sdkUrl === 'http://localhost:35419/';

    this.realCwd = process.cwd();
    this.citizen = path.normalize(GetConvar('citizen_path'));

    this.cfxLocalAppData = path.join(process.env.LOCALAPPDATA, 'citizenfx');

    // __dirname will be like `resources/sdk-root/shell/build_server/index.js`
    this.sdkResources = path.join(__dirname, '../../..');

    this.sdkRoot = path.join(this.sdkResources, 'sdk-root');
    this.sdkGame = path.join(this.sdkResources, 'sdk-game');

    this.sdkRootShell = path.join(this.sdkRoot, 'shell');
    this.sdkRootShellBuild = path.join(this.sdkRootShell, 'build');

    // Also defined at personality-theia/fxdk-project/src/backend/rebindEnvVariablesServerImpl.ts
    this.theiaConfigPath = path.join(this.cfxLocalAppData, 'sdk-personality-theia');
    this.theiaPluginsPath = path.join(this.cfxLocalAppData, 'sdk-personality-theia-plugins');

    this.sdkRootTheia = path.join(this.sdkRoot, 'personality-theia');
    this.sdkRootTheiaArchive = path.join(this.sdkRoot, 'personality-theia.tar');

    this.sdkStorage = path.join(this.cfxLocalAppData, 'sdk-storage');

    this.serverArtifacts = path.join(this.sdkStorage, 'artifacts');
    this.serverContainer = path.join(this.sdkStorage, 'server');
    this.serverDataPath = path.join(this.sdkStorage, 'serverData');

    this.nativesDocluaPath = path.join(this.cfxLocalAppData, 'natives-doclua');
    this.recentProjectsFilePath = path.join(this.sdkStorage, 'recent-projects.json');
    this.featuresFilePath = path.join(this.sdkStorage, 'features.json');

    this.wellKnownPathsPath = path.join(this.sdkStorage, 'well-known-paths.json');

    this.systemResourcesRoot = path.join(this.sdkStorage, 'system-resources');
    this.systemResourcesPath = path.join(this.systemResourcesRoot, 'resources');
  }
}
