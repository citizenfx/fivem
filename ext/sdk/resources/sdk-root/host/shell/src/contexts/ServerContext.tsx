import React from 'react';
import { ServerStates } from '../sdkApi/api.types';
import { serverApi } from '../sdkApi/events';
import { getEnabledResourcesPaths } from '../sdkApi/utils';
import { sendApiMessage } from '../utils/api';
import { useApiMessage } from '../utils/hooks';
import { sendCommand } from '../utils/sendCommand';
import { ProjectContext } from './ProjectContext';
import { StateContext } from './StateContext';


export interface ServerContext {
  serverState: ServerStates,
  serverOutput: string,

  clientConnected: boolean,

  resourcesState: {
    [name: string]: boolean,
  },

  startServer: () => void,
  stopServer: () => void,
}
export const ServerContext = React.createContext<ServerContext>({
  serverState: ServerStates.down,
  serverOutput: '',

  clientConnected: false,

  resourcesState: {},

  startServer: () => {},
  stopServer: () => {},
});

export const ServerContextProvider = React.memo(({ children }) => {
  const { project, projectResources } = React.useContext(ProjectContext);
  const { gameLaunched } = React.useContext(StateContext);

  const [serverState, setServerState] = React.useState<ServerStates>(ServerStates.down);
  const [serverOutput, setServerOutput] = React.useState<string>('');

  const [clientConnected, setClientConnected] = React.useState(false);
  const connectPending = React.useRef(false);

  const [resourcesState, setResourcesState] = React.useState({});

  const startServer = React.useCallback(() => {
    if (project) {
      const enabledResourcesPaths = getEnabledResourcesPaths(project, projectResources);

      sendApiMessage(serverApi.start, {
        projectPath: project.path,
        enabledResourcesPaths,
      });

      connectPending.current = true;
    }
  }, [project, projectResources]);

  const stopServer = React.useCallback(() => {
    sendApiMessage(serverApi.stop);
  }, []);

  // Handling boot
  React.useEffect(() => {
    sendApiMessage(serverApi.ackState);
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

      if (clientConnected) {
        sendCommand('disconnect');
        setClientConnected(false);
      }
    }
  }, [serverState, gameLaunched, clientConnected, setResourcesState, setClientConnected]);

  // Handling game events
  React.useEffect(() => {
    const handleMessage = (e) => {
      if (e.data.type === 'connected') {
        console.log('CLIENT IS CONNECTED');
        setClientConnected(true);
      }
    };

    window.addEventListener('message', handleMessage);

    return () => window.removeEventListener('message', handleMessage);
  }, [setClientConnected]);

  // Handling resources changes
  React.useEffect(() => {
    if (project && serverState === ServerStates.up) {
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

    clientConnected,

    resourcesState,

    startServer,
    stopServer,
  };

  return (
    <ServerContext.Provider value={value}>
      {children}
    </ServerContext.Provider>
  );
});
