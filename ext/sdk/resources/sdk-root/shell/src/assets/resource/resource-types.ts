import { ProjectAssetBaseConfig } from "shared/project.types";

export interface ResourceCommandStatus {
  running: boolean,
  outputChannelId: string,
}

export interface ResourceStatus {
  watchCommands: Record<string, ResourceCommandStatus>,
}

export interface ResourceAssetConfig extends ProjectAssetBaseConfig {
  restartOnChange: boolean,
}
