import { combineVisibilityFilters, VisibilityFilter, visibilityFilters } from 'components/Explorer/Explorer.filters';
import * as React from 'react';


export const ProjectExplorerVisibilityFilter = combineVisibilityFilters(
  visibilityFilters.hideDotFilesAndDirs,
);

export interface ProjectExplorerItemContext {
  disableDirectoryCreate: boolean,
  disableDirectoryDelete: boolean,
  disableDirectoryRename: boolean,

  disableFileOpen: boolean,
  disableFileCreate: boolean,
  disableFileDelete: boolean,
  disableFileRename: boolean,

  disableEntryMove: boolean,

  disableAssetCreate: boolean,
  disableAssetRename: boolean,
  disableAssetDelete: boolean,

  visibilityFilter: VisibilityFilter,
}

const defaultValues: ProjectExplorerItemContext = {
  disableDirectoryCreate: false,
  disableDirectoryDelete: false,
  disableDirectoryRename: false,
  disableFileOpen: false,
  disableFileCreate: false,
  disableFileDelete: false,
  disableFileRename: false,
  disableEntryMove: false,
  disableAssetCreate: false,
  disableAssetRename: false,
  disableAssetDelete: false,
  visibilityFilter: ProjectExplorerVisibilityFilter,
};

export const ProjectExplorerItemContext = React.createContext<ProjectExplorerItemContext>(defaultValues);

export interface ProjectExplorerItemContextProviderProps {
  options: Partial<ProjectExplorerItemContext>,
  children: React.ReactNode,
}

export const ProjectExplorerItemContextProvider = React.memo(function ProjectExplorerItemContextProvider({ options, children }: ProjectExplorerItemContextProviderProps) {
  const parentValues = React.useContext(ProjectExplorerItemContext);

  const value = {
    ...parentValues,
    ...options,
  };

  return (
    <ProjectExplorerItemContext.Provider value={value}>
      {children}
    </ProjectExplorerItemContext.Provider>
  );
});
