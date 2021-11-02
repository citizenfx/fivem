import { ProjectAssetBaseConfig } from "./project.types";

export namespace APIRQ {
  export interface ProjectCreate {
    projectName: string,
    projectPath: string,
  }

  export interface ProjectSetAssetConfig<AssetConfigType extends ProjectAssetBaseConfig = ProjectAssetBaseConfig> {
    assetPath: string,
    config: Partial<AssetConfigType>,
  }

  export interface ProjectCreateDirectory {
    directoryPath: string,
    directoryName: string,
  }

  export interface ProjectCreateFile {
    filePath: string,
    fileName: string,
  }

  export interface FetchReleases {
    repoUrl: string,
  }

  export interface ReleaseInfo {
    name: string,
    body: string,
    createdAt: string,
    downloadUrl: string,
  }

  export interface FetchReleasesResponse {
    success: boolean,
    releases: ReleaseInfo[],
  }

  export interface AssetRename {
    assetPath: string,
    newAssetName: string,
  }

  export interface ProjectStartServer {
    licenseKey?: string,
    steamWebApiKey?: string,
    tebexSecret?: string,
  }

  export interface RenameEntry {
    entryPath: string,
    newName: string,
  }
  export interface EntryRenamed {
    fromEntryPath: string,
    toEntryPath: string,
  }

  export interface MoveEntry {
    sourcePath: string,
    targetPath: string,
  }
  export interface EntryMoved {
    fromEntryPath: string,
    toEntryPath: string,
  }

  export interface DeleteEntry {
    entryPath: string,
    hardDelete?: boolean,
  }
  export interface EntryDeleted {
    entryPath: string,
  }
  export enum DeleteEntryResponse {
    Ok,
    FailedToRecycle,
  }

  export interface CopyEntries {
    sourcePaths: string[];
    targetPath: string
  }

  export type CopyEntry = MoveEntry;

  export interface ProjectBuild {
    buildPath: string,
    useVersioning: boolean,
    deployArtifact: boolean,
    steamWebApiKey: string,
    tebexSecret: string,
  }

  export interface WorldEditorStart {
    mapPath: string,
  }
}
