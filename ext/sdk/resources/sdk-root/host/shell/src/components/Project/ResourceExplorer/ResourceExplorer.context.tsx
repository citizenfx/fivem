import * as React from 'react';
import { ProjectContext } from '../../../contexts/ProjectContext';
import { ProjectPathsState } from '../../../sdkApi/api.types';
import { projectApi } from '../../../sdkApi/events';
import { sendApiMessage } from '../../../utils/api';
import { useApiMessage } from '../../../utils/hooks';


export interface ResourceExplorerContext {
  pathsState: ProjectPathsState,

  setPathState: (path: string, expanded: boolean) => void;
}

const defaultValues: ResourceExplorerContext = {
  pathsState: {},

  setPathState: () => {},
};

export const ResourceExplorerContext = React.createContext<ResourceExplorerContext>(defaultValues);

export const ResourceExplorerContextProvider = React.memo(({ children }) => {
  const { project } = React.useContext(ProjectContext);
  const [pathsState, setPathsState] = React.useState<ProjectPathsState>(defaultValues.pathsState);

  React.useEffect(() => {
    if (project) {
      setPathsState(project.manifest.pathsState);
    }
  }, [project?.manifest.pathsState]);

  useApiMessage(projectApi.pathsState, (remotePathsState: ProjectPathsState) => {
    setPathsState(remotePathsState);
  }, [setPathsState]);

  const setPathState = React.useCallback((path: string, expanded: boolean) => {
    const newPathsState = {
      ...pathsState,
      [path]: expanded,
    };

    setPathsState(newPathsState);

    sendApiMessage(projectApi.setPathsState, newPathsState);
  }, [pathsState, setPathsState]);

  const value = {
    pathsState,
    setPathState,
  };

  return (
    <ResourceExplorerContext.Provider value={value}>
      {children}
    </ResourceExplorerContext.Provider>
  );
});
