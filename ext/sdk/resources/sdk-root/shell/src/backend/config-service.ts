import { injectable } from "inversify";
import path from 'path';
import { URL } from 'url';

if (!process.env.LOCALAPPDATA) {
  console.error('No LOCALAPPDATA env variable');
  process.exit(-1);
}

@injectable()
export class ConfigService {
  readonly shellBackendPort: number;
  readonly editorBackendPort: number;

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

  readonly sdkRootFXCode: string;
  readonly sdkRootFXCodeArchive: string;

  readonly fxcodeDataPath: string;

  readonly sdkStorage: string;
  readonly serverArtifacts: string;
  readonly serverContainer: string;
  readonly serverDataPath: string;

  readonly systemResourcesRoot: string;
  readonly systemResourcesPath: string;

  readonly recentProjectsFilePath: string;
  readonly featuresFilePath: string;

  readonly wellKnownPathsPath: string;

  readonly archetypesCollectionPath: string;

  constructor() {
    const { LOCALAPPDATA } = process.env;

    if (!LOCALAPPDATA) {
      throw new Error('No LOCALAPPDATA in env variables');
    }

    this.sdkUrl = GetConvar('sdk_url');
    this.selfHosted = this.sdkUrl === 'http://localhost:35419/';

    this.shellBackendPort = parseInt(new URL(this.sdkUrl).port, 10) || 80;
    this.editorBackendPort = this.shellBackendPort + 1;

    this.realCwd = process.cwd();
    this.citizen = path.normalize(GetConvar('citizen_path'));

    process.env.CITIZEN_PATH = this.citizen;

    this.cfxLocalAppData = path.join(LOCALAPPDATA, 'citizenfx');

    // __dirname will be like `resources/sdk-root/shell/build_server/index.js`
    this.sdkResources = path.join(__dirname, '../../..');

    this.sdkRoot = path.join(this.sdkResources, 'sdk-root');
    this.sdkGame = path.join(this.sdkResources, 'sdk-game');

    this.sdkRootShell = path.join(this.sdkRoot, 'shell');
    this.sdkRootShellBuild = path.join(this.sdkRootShell, 'build');

    this.fxcodeDataPath = path.join(this.cfxLocalAppData, 'sdk-personality-fxcode');

    this.sdkRootFXCode = path.join(this.sdkRoot, 'fxcode');
    this.sdkRootFXCodeArchive = path.join(this.sdkRoot, 'fxcode.tar');

    this.sdkStorage = path.join(this.cfxLocalAppData, 'sdk-storage');

    this.serverArtifacts = path.join(this.sdkStorage, 'artifacts');
    this.serverContainer = path.join(this.sdkStorage, 'server');
    this.serverDataPath = path.join(this.sdkStorage, 'serverData');

    this.recentProjectsFilePath = path.join(this.sdkStorage, 'recent-projects.json');
    this.featuresFilePath = path.join(this.sdkStorage, 'features.json');

    this.wellKnownPathsPath = path.join(this.sdkStorage, 'well-known-paths.json');

    this.systemResourcesRoot = path.join(this.sdkStorage, 'system-resources');
    this.systemResourcesPath = path.join(this.systemResourcesRoot, 'resources');

    this.archetypesCollectionPath = path.join(this.cfxLocalAppData, 'archetypes.json');
  }
}
