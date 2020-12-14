import * as React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { FilesystemEntry, ProjectPathsState } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { useApiMessage } from 'utils/hooks';
import { CopyEntryRequest, MoveEntryRequest } from 'shared/api.requests';


export enum EntryRelocateOperation {
  Copy,
  Move,
}

export interface ProjectExplorerContext {
  pathsState: ProjectPathsState,
  setPathState: (path: string, expanded: boolean) => void,

  relocateSourceEntry: FilesystemEntry | void,
  relocateOperation: EntryRelocateOperation | void,
  setRelocationContext: (entry: FilesystemEntry, operation: EntryRelocateOperation) => void,
  resetRelocationContext: () => void,
  applyRelocation: (target: FilesystemEntry) => void,
}

const defaultValues: ProjectExplorerContext = {
  pathsState: {},
  setPathState: () => {},

  relocateSourceEntry: undefined,
  relocateOperation: undefined,
  setRelocationContext: () => {},
  resetRelocationContext: () => {},
  applyRelocation: () => {},
};

export const ProjectExplorerContext = React.createContext<ProjectExplorerContext>(defaultValues);

export const ProjectExplorerContextProvider = React.memo(function ProjectExplorerContextProvider({ children }) {
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

    sendApiMessage(projectApi.setPathsStatePatch, { [path]: expanded });
  }, [pathsState, setPathsState]);

  const [relocateSourceEntry, setRelocateSourceEntry] = React.useState<FilesystemEntry | void>(undefined);
  const [relocateOperation, setRelocateOperation] = React.useState<EntryRelocateOperation | undefined>(undefined);

  const setRelocationContext = React.useCallback((entry: FilesystemEntry, operation: EntryRelocateOperation) => {
    console.log('Setting relocation context', {
      entry,
      operation,
    });

    setRelocateSourceEntry(entry);
    setRelocateOperation(operation);
  }, [setRelocateSourceEntry, setRelocateOperation]);

  const resetRelocationContext = React.useCallback(() => {
    setRelocateSourceEntry(undefined);
    setRelocateOperation(undefined);
  }, [setRelocateSourceEntry, setRelocateOperation]);

  const applyRelocation = React.useCallback((target: FilesystemEntry) => {
    setRelocateSourceEntry(undefined);
    setRelocateOperation(undefined);

    if (!relocateSourceEntry) {
      return;
    }

    switch (relocateOperation) {
      case EntryRelocateOperation.Move: {
        const request: MoveEntryRequest = {
          sourcePath: relocateSourceEntry.path,
          targetPath: target.path,
        };

        return sendApiMessage(projectApi.moveEntry, request);
      }
      case EntryRelocateOperation.Copy: {
        const request: CopyEntryRequest = {
          sourcePath: relocateSourceEntry.path,
          targetPath: target.path,
        };

        return sendApiMessage(projectApi.copyEntry, request);
      }
    }
  }, [setRelocateSourceEntry, setRelocateOperation, relocateSourceEntry, relocateOperation]);

  const value = {
    pathsState,
    setPathState,

    relocateSourceEntry,
    relocateOperation,
    setRelocationContext,
    resetRelocationContext,
    applyRelocation,
  };

  return (
    <ProjectExplorerContext.Provider value={value}>
      {children}
    </ProjectExplorerContext.Provider>
  );
});
