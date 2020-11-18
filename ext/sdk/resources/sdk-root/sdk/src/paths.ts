import * as path from 'path';

if (!process.env.LOCALAPPDATA) {
  console.error('No LOCALAPPDATA env variable');
  process.exit(-1);
}

export const realCwd = process.cwd();
export const citizen = path.normalize(GetConvar('citizen_path'));

export const sdkResources = path.join(__dirname, '../../..');

export const sdkRoot = path.join(sdkResources, 'sdk-root');
export const sdkGame = path.join(sdkResources, 'sdk-game');

export const sdkRootHost = path.join(sdkRoot, 'host');
export const sdkRootShell = path.join(sdkRootHost, 'shell');
export const sdkRootTheia = path.join(sdkRootHost, 'personality-theia');
export const sdkRootTheiaArchive = path.join(sdkRootHost, 'personality-theia.tar');

export const localAppData = path.join(process.env.LOCALAPPDATA, 'citizenfx/sdk-storage');
export const serverArtifacts = path.join(localAppData, 'artifacts');
export const serverContainer = path.join(localAppData, 'server');
export const serverDataPath = path.join(localAppData, 'serverData');

export const recentProjectsFilePath = path.join(localAppData, 'recent-projects.json');

export const wellKnownPathsPath = path.join(localAppData, 'well-known-paths.json');
