import { FilesystemEntry } from "shared/api.types";

export type VisibilityFilter = (entry: FilesystemEntry) => boolean;

export const combineVisibilityFilters = (...filters: VisibilityFilter[]): VisibilityFilter => (entry: FilesystemEntry) => {
  return filters.every((filter) => filter(entry));
};

export const visibilityFilters = {
  hideFiles: (entry: FilesystemEntry) => !entry.isFile,
  hideAssets: (entry: FilesystemEntry) => !entry.meta.assetMeta,
  hideDotFilesAndDirs: (entry: FilesystemEntry) => !entry.name.startsWith('.'),
  hideResources: (entry: FilesystemEntry) => !entry.meta.isResource,
  hideProjects: (entry: FilesystemEntry) => !entry.meta.isFxdkProject,
};

export const defaultVisibilityFilter: VisibilityFilter = () => true;
