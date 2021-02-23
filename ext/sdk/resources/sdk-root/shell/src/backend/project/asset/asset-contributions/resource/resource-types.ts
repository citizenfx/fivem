export interface ResourceCommandStatus {
  running: boolean,
  outputChannelId: string,
}

export interface ResourceStatus {
  watchCommands: Record<string, ResourceCommandStatus>,
}
