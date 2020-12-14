export const projectApi = {
  checkCreateRequest: 'project:checkCreateRequest',
  checkCreateResult: 'project:checkCreateResult',
  create: 'project:create',
  open: 'project:open',
  update: 'project:update',
  setServerUpdateChannel: 'project:setServerUpdateChannel',

  getRecents: 'project:getRecents',
  recents: 'project:recents',
  removeRecent: 'project:removeRecent',

  setPathsState: 'project:setPathsState',
  setPathsStatePatch: 'project:setPathsStatePatch',
  pathsState: 'project:pathsState',

  createResource: 'project:createResource',
  resourceAdded: 'project:addResource',
  renameResource: 'project:renameResource',
  deleteResource: 'project:deleteResource',
  updateResources: 'project:updateResources',
  setResourceConfig: 'project:setResourceConfig',

  fsUpdate: 'project:fsUpdate',

  createDirectory: 'project:createDirectory',
  renameDirectory: 'project:renameDirectory',
  deleteDirectory: 'project:deleteDirectory',

  createFile: 'project:createFile',
  renameFile: 'project:renameFile',
  deleteFile: 'project:deleteFile',

  moveEntry: 'project:moveEntry',
  copyEntry: 'project:copyEntry',
};

export const assetApi = {
  create: 'asset:create',
  rename: 'asset:rename',
  delete: 'asset:delete',

  git: {
    import: 'asset:git:import',
    update: 'asset:git:update',
  },
};

export const stateApi = {
  ackState: 'ackState',
  state: 'state',
  serverDataState: 'serverDataState',
  gameLaunched: 'state:gameLaunched',
};

export const explorerApi = {
  readDirRecursive: 'explorer:readDirRecursive',
  readDir: 'explorer:readDir',
  readRoots: 'explorer:readRoots',
  readRoot: 'explorer:readRoot',

  dir: 'explorer:dir',
  dirRecursive: 'explorer:dirRecursive',
  roots: 'explorer:roots',
  root: 'explorer:root',
  newDir: 'explorer:newDir',
  createDir: 'explorer:createDir',
};

export const serverApi = {
  ackState: 'server:ackState',
  state: 'server:state',

  start: 'server:start',
  stop: 'server:stop',

  output: 'server:output',
  clearOutput: 'server:clearOutput',
  sendCommand: 'server:sendCommand',

  ackResourcesState: 'server:ackResourcesState',
  resourcesState: 'server:resourcesState',

  setEnabledResources: 'server:refreshResources',
  restartResource: 'server:restartResource',
  stopResource: 'server:stopResource',
  startResource: 'server:startResource',

  ackInstallationState: 'server:ackInstallationState',
  installationState: 'server:installationState',
  updateChannelsState: 'server:updateChannelsState',
  ackUpdateChannelsState: 'server:ackUpdateChannelsState',
  checkForUpdates: 'server:checkForUpdates',
  installUpdate: 'server:installUpdate',
};

export const statusesApi = {
  ack: 'statuses:ack',
  update: 'statuses:update',
  statuses: 'statuses:statuses',
};

export const notificationsApi = {
  ack: 'notifications:ack',
  create: 'notifications:create',
  delete: 'notifications:delete',
};
