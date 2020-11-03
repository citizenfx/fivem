import React from 'react';
import { ServerInstallationState, ServerStates, ServerUpdateChannel, ServerUpdateChannelsState } from '../sdkApi/api.types';
import { serverApi } from '../sdkApi/events';
import { getEnabledResourcesPaths } from '../sdkApi/utils';
import { sendApiMessage } from '../utils/api';
import { useApiMessage } from '../utils/hooks';
import { sendCommand } from '../utils/sendCommand';
import { ProjectContext } from './ProjectContext';
import { StateContext } from './StateContext';


export interface ServerContext {
  serverState: ServerStates | null,
  serverOutput: string,

  updateChannelsState: ServerUpdateChannelsState,
  installationState: ServerInstallationState,

  clientConnected: boolean,

  resourcesState: {
    [name: string]: boolean,
  },

  startServer: () => void,
  stopServer: () => void,

  checkForUpdates: (updateChannel: ServerUpdateChannel) => void,
  installUpdate: (updateChannel: ServerUpdateChannel) => void,
}
export const ServerContext = React.createContext<ServerContext>({
  serverState: ServerStates.down,
  serverOutput: '',

  updateChannelsState: {},
  installationState: {},

  clientConnected: false,

  resourcesState: {},

  startServer: () => {},
  stopServer: () => {},

  checkForUpdates: () => {},
  installUpdate: () => {},
});

export const ServerContextProvider = React.memo(function ServerContextProvider({ children }) {
  const { project, projectResources } = React.useContext(ProjectContext);
  const { gameLaunched } = React.useContext(StateContext);

  const [serverState, setServerState] = React.useState<ServerStates | null>(null);
  const [serverOutput, setServerOutput] = React.useState<string>('');

  const [updateChannelsState, setUpdateChannelsState] = React.useState<ServerUpdateChannelsState>({});
  const [installationState, setInstallationState] = React.useState<ServerInstallationState>({});

  const [clientConnected, setClientConnected] = React.useState(false);
  const connectPending = React.useRef(false);

  const [resourcesState, setResourcesState] = React.useState({});

  const startServer = React.useCallback(() => {
    if (project) {
      const enabledResourcesPaths = getEnabledResourcesPaths(project, projectResources);

      sendApiMessage(serverApi.start, {
        projectPath: project.path,
        updateChannel: project.manifest.serverUpdateChannel,
        enabledResourcesPaths,
      });

      connectPending.current = true;
    }
  }, [project, projectResources]);

  const stopServer = React.useCallback(() => {
    sendApiMessage(serverApi.stop);
  }, []);

  const checkForUpdates = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(serverApi.checkForUpdates, updateChannel);
  }, []);

  const installUpdate = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(serverApi.installUpdate, updateChannel);
  }, []);

  // Handling boot
  React.useEffect(() => {
    sendApiMessage(serverApi.ackState);
    sendApiMessage(serverApi.ackUpdateChannelsState);
    sendApiMessage(serverApi.ackInstallationState);

    // Ack sdk-game state
    sendCommand('sdk:ackConnected');
  }, []);

  // Handling server state
  React.useEffect(() => {
    if (serverState === ServerStates.up) {
      sendApiMessage(serverApi.ackResourcesState);

      if (gameLaunched && connectPending.current && !clientConnected) {
        console.log('CONNECTING TO THE SERVER');
        connectPending.current = false;
        sendCommand('connect 127.0.0.1:30120');
      }
    }
    if (serverState === ServerStates.down) {
      setResourcesState({});
      sendCommand('disconnect');
      setClientConnected(false);
    }
  }, [serverState, gameLaunched, clientConnected, setResourcesState, setClientConnected]);

  // Handling game events
  React.useEffect(() => {
    const handleMessage = (e) => {
      if (e.data.type === 'connected') {
        setClientConnected(true);
      }
    };

    window.addEventListener('message', handleMessage);

    return () => window.removeEventListener('message', handleMessage);
  }, [setClientConnected]);

  // Handling project changes
  React.useEffect(() => {
    if (!project) {
      return;
    }

    if (serverState === ServerStates.up) {
      const enabledResourcesPaths = getEnabledResourcesPaths(project, projectResources);

      sendApiMessage(serverApi.refreshResources, {
        projectPath: project.path,
        enabledResourcesPaths,
      });
    }
  }, [project, projectResources, serverState]);

  useApiMessage(serverApi.state, (state: ServerStates) => {
    setServerState(state);
  }, [setServerState]);

  useApiMessage(serverApi.output, (chunks: string) => {
    setServerOutput(serverOutput + chunks);
  }, [setServerOutput, serverOutput]);

  useApiMessage(serverApi.updateChannelsState, (newUpdateChannelsState) => {
    setUpdateChannelsState(newUpdateChannelsState);
  });

  useApiMessage(serverApi.installationState, (newInstallationState) => {
    setInstallationState(newInstallationState);
  });

  useApiMessage(serverApi.clearOutput, () => {
    setServerOutput('');
  }, [setServerOutput]);

  useApiMessage(serverApi.resourcesState, (newResourcesState) => {
    console.log('#### Resources state', newResourcesState);
    setResourcesState(newResourcesState);
  }, [setResourcesState]);

  const value = {
    serverState,
    serverOutput,

    updateChannelsState,
    installationState,

    clientConnected,

    resourcesState,

    startServer,
    stopServer,

    checkForUpdates,
    installUpdate,
  };

  return (
    <ServerContext.Provider value={value}>
      {children}
    </ServerContext.Provider>
  );
});
