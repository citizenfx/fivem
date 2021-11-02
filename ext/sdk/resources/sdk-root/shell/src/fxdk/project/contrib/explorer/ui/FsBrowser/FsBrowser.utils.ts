import { IFsEntry } from "fxdk/project/common/project.types";

export namespace FsBrowserUtils {
  export type IVisibileFilter = (entry: IFsEntry) => boolean;

  export const combineSelectableFilters = (...filters: IVisibileFilter[]): IVisibileFilter => (entry: IFsEntry) => {
    return filters.every((filter) => filter(entry));
  };

  export const filters = {
    discardFiles: (entry: IFsEntry) => entry.isDirectory,
    discardAssets: (entry: IFsEntry) => entry.handle === 'directory' || entry.handle === 'file',
    discardDotFilesAndDirs: (entry: IFsEntry) => !entry.name.startsWith('.'),
  };

  export const getRelativePath = (basePath: string, path: string): string => {
    return path.replace(basePath, '').replace('\\', '').replace('/', '');
  }
}
