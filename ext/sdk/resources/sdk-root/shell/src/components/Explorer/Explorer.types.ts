import { FilesystemEntry } from "shared/api.types";
import { VisibilityFilter } from "./Explorer.filters";

export interface BaseExplorerProps {
  className?: string,
  hideFiles?: boolean,
  disableFiles?: boolean,
  selectedPath?: string | void,
  onSelectPath?: (path: string, entry: FilesystemEntry) => void,
  selectableFilter?: (entry: FilesystemEntry) => boolean,
  visibilityFilter?: VisibilityFilter,
  basePath?: string,
  loadAllRecursively?: boolean,
}

export interface ExplorerDirProps extends BaseExplorerProps {
  childs?: FilesystemEntry[],
  childsCache: {
    [key: string]: FilesystemEntry[],
  },

  dir: FilesystemEntry,
  open: boolean,

  loadDir: (dir: string) => void,
  updateBeacon: {},
}
