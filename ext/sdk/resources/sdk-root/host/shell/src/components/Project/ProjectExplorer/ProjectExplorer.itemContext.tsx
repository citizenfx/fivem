import * as React from 'react';
import { combineVisibilityFilters, VisibilityFilter, visibilityFilters } from '../../Explorer/Explorer';


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

export const ProjectExplorerItemContextProvider = React.memo(({ options, children }: ProjectExplorerItemContextProviderProps) => {
  const value = {
    ...defaultValues,
    ...options,
  };

  return (
    <ProjectExplorerItemContext.Provider value={value}>
      {children}
    </ProjectExplorerItemContext.Provider>
  );
});
