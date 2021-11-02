import { apiEndpointsGroup } from "./api.protocol";

export const githubApi = apiEndpointsGroup({
  fetchReleases: 'github:fetchReleases',
});

export const stateApi = apiEndpointsGroup({
  ackState: 'ackState',
  state: 'state',
  serverDataState: 'serverDataState',
  gameLaunched: 'state:gameLaunched',
  setUserId: 'state:setUserId',
});

export const explorerApi = apiEndpointsGroup({
  readDirRecursive: 'explorer:readDirRecursive',
  readDir: 'explorer:readDir',

  dir: 'explorer:dir',
  dirRecursive: 'explorer:dirRecursive',
});

export const serverApi = apiEndpointsGroup({
  ackState: 'server:ackState',
  state: 'server:state',

  resourceDatas: 'server:resourceDatas',
  bufferedOutput: 'server:output',
  structuredOutputMessage: 'server:outputStructured',
  clearOutput: 'server:clearOutput',
  sendCommand: 'server:sendCommand',

  ackResourcesState: 'server:ackResourcesState',
  resourcesState: 'server:resourcesState',

  restartResource: 'server:restartResource',
  stopResource: 'server:stopResource',
  startResource: 'server:startResource',

  updateChannelsState: 'server:updateChannelsState',
  ackUpdateChannelsState: 'server:ackUpdateChannelsState',
  checkForUpdates: 'server:checkForUpdates',
  installUpdate: 'server:installUpdate',
});

export const statusesApi = apiEndpointsGroup({
  ack: 'statuses:ack',
  update: 'statuses:update',
  statuses: 'statuses:statuses',
});

export const notificationsApi = apiEndpointsGroup({
  ack: 'notifications:ack',
  create: 'notifications:create',
  delete: 'notifications:delete',
});

export const taskReporterApi = apiEndpointsGroup({
  ackTasks: 'taskReporter:ackTasks',
  tasks: 'taskReporter:tasks',
  taskAdded: 'taskReporter:taskAdded',
  taskChanged: 'taskReporter:taskChanged',
  taskDeleted: 'taskReporter:taskDeleted',
});

export const outputApi = apiEndpointsGroup({
  output: 'output:output',
  outputLabel: 'output:outputLabel',
  flush: 'output:flush',
});

export const gameApi = apiEndpointsGroup({
  ack: 'game:ack',
  gameState: 'game:gameState',
  gameLaunched: 'game:gameLaunched',
  gameProcessStateChanged: 'game:gameStateChanged',
  connectionStateChanged: 'game:connectionStateChanged',
  start: 'game:start',
  stop: 'game:stop',
  restart: 'game:restart',
  refreshArchetypesCollection: 'game:refreshArchetypesCollection',
});

export const worldEditorApi = apiEndpointsGroup({
  start: 'we:start',
  stop: 'we:stop',
  stopped: 'we:stopped',

  mapLoaded: 'we:mapLoaded',
  setCam: 'we:setCam',
  createPatch: 'we:createPatch',
  applyPatchChange: 'we:applyPatchChange',
  deletePatch: 'we:deletePatch',
  createAddition: 'we:createAddition',
  additionPlaced: 'we:additionPlaced',
  deleteAddition: 'we:deleteAddition',
  setAddition: 'we:setAddition',
  applyAdditionChange: 'we:applyAdditionChange',
  setAdditionGroup: 'we:setAdditionGroup',
  createAdditionGroup: 'we:createAdditionGroup',
  deleteAdditionGroup: 'we:deleteAdditionGroup',
  setAdditionGroupLabel: 'we:setAdditionGroupName',
});
