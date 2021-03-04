// Mirror of SDKGameProcessManager::GameProcessState
export enum SDKGameProcessState {
  GP_STOPPED,
  GP_STARTING,
  GP_RUNNING,
  GP_STOPPING,
}

// Mirror of NetLibrary::ConnectionState
export enum NetLibraryConnectionState {
  CS_IDLE,
  CS_INITING,
  CS_INITRECEIVED,
  CS_DOWNLOADING,
  CS_DOWNLOADCOMPLETE,
  CS_FETCHING,
  CS_CONNECTING,
  CS_CONNECTED,
  CS_ACTIVE
}
