import * as React from 'react';
import { observer } from 'mobx-react-lite';
import { FilesystemEntry } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { APIRQ } from 'shared/api.requests';


export enum EntryRelocateOperation {
  Copy,
  Move,
}

export interface ProjectExplorerContext {
  relocateSourceEntry: FilesystemEntry | void,
  relocateOperation: EntryRelocateOperation | void,
  setRelocationContext: (entry: FilesystemEntry, operation: EntryRelocateOperation) => void,
  resetRelocationContext: () => void,
  applyRelocation: (target: FilesystemEntry) => void,
}

const defaultValues: ProjectExplorerContext = {
  relocateSourceEntry: undefined,
  relocateOperation: undefined,
  setRelocationContext: () => {},
  resetRelocationContext: () => {},
  applyRelocation: () => {},
};

export const ProjectExplorerContext = React.createContext<ProjectExplorerContext>(defaultValues);

export const ProjectExplorerContextProvider = observer(function ProjectExplorerContextProvider({ children }) {
  const [relocateSourceEntry, setRelocateSourceEntry] = React.useState<FilesystemEntry | void>(undefined);
  const [relocateOperation, setRelocateOperation] = React.useState<EntryRelocateOperation | undefined>(undefined);

  const setRelocationContext = React.useCallback((entry: FilesystemEntry, operation: EntryRelocateOperation) => {
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
        const request: APIRQ.MoveEntry = {
          sourcePath: relocateSourceEntry.path,
          targetPath: target.path,
        };

        return sendApiMessage(projectApi.moveEntry, request);
      }
      case EntryRelocateOperation.Copy: {
        const request: APIRQ.CopyEntry = {
          sourcePath: relocateSourceEntry.path,
          targetPath: target.path,
        };

        return sendApiMessage(projectApi.copyEntry, request);
      }
    }
  }, [setRelocateSourceEntry, setRelocateOperation, relocateSourceEntry, relocateOperation]);

  const value = {
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
