import React from 'react';
import classnames from 'classnames';
import { FilesystemEntry, FilesystemEntryMap } from 'shared/api.types';
import { closedDirectoryIcon, fileIcon, openDirectoryIcon, projectIcon, resourceIcon } from 'constants/icons';
import { sendApiMessage } from 'utils/api';
import { explorerApi } from 'shared/events';
import { useApiMessage } from 'utils/hooks';
import s from './Explorer.module.scss';


export type VisibilityFilter = (entry: FilesystemEntry) => boolean;

export const combineVisibilityFilters = (...filters: VisibilityFilter[]): VisibilityFilter => (entry: FilesystemEntry) => {
  return filters.every((filter) => filter(entry));
};

export const visibilityFilters = {
  hideFiles: (entry: FilesystemEntry) => !entry.isFile,
  hideAssets: (entry: FilesystemEntry) => !entry.meta.assetMeta,
  hideDotFilesAndDirs: (entry: FilesystemEntry) => !entry.name.startsWith('.'),
};

export const getRelativePath = (basePath: string, path: string): string => {
  return path.replace(basePath, '').replace('\\', '').replace('/', '');
}

export const defaultVisibilityFilter: VisibilityFilter = () => true;
const defaultSelectableFilter = () => true;
const noop = () => { };

const getEntryIcon = (entry: FilesystemEntry, open: boolean) => {
  if (entry.meta.isFxdkProject) {
    return projectIcon;
  }

  if (entry.meta.isResource) {
    return resourceIcon;
  }

  if (entry.isDirectory) {
    return open ? openDirectoryIcon : closedDirectoryIcon;
  } else {
    return fileIcon;
  }
}

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

interface ExplorerDirProps extends BaseExplorerProps {
  childs?: FilesystemEntry[],
  childsCache: {
    [key: string]: FilesystemEntry[],
  },

  dir: FilesystemEntry,
  open: boolean,

  loadDir: (dir: string) => void,
  updateBeacon: {},
}

/**
 * Explorer dir component
 */
const ExplorerDir = React.memo(function ExplorerDir(props: ExplorerDirProps) {
  const {
    childs,
    childsCache,
    dir,
    selectedPath,
    onSelectPath = noop,
    loadDir,
    selectableFilter = defaultSelectableFilter,
    visibilityFilter = defaultVisibilityFilter,
  } = props;

  // Open by default if props.open or if selectedPath starts with current dir
  const [open, setOpen] = React.useState(props.open || (selectedPath || '').startsWith(dir.path));
  const toggleOpen = React.useCallback(() => {
    setOpen(!open);

    if (!childs) {
      loadDir(dir.path);
    }
  }, [dir, open, childs, loadDir]);

  const handleSelectDir = React.useCallback(() => {
    if (selectableFilter(dir)) {
      onSelectPath(dir.path, dir);
    }
  }, [dir, onSelectPath, selectableFilter]);

  // FIXME: OLD MECHANISM, USE visibilityFilter instead
  if (dir.isFile && props.hideFiles) {
    return null;
  }

  if (!visibilityFilter(dir)) {
    return null;
  }

  const childsNodes = childs
    ? childs.map((childDir) => (
      <ExplorerDir
        key={childDir.path}
        {...props}
        open={false}
        dir={childDir}
        childs={childsCache[childDir.path]}
      />
    ))
    : null;

  const nameClassName = classnames(s.name, {
    [s.selected]: selectedPath === dir.path,
  });

  const icon = getEntryIcon(dir, open);

  return (
    <div className={s.dir}>
      <div className={nameClassName} onClick={handleSelectDir} onDoubleClick={toggleOpen}>
        {icon} {props.dir.name}
      </div>

      {childsNodes && open && (
        <div className={s.children}>
          {childsNodes}
        </div>
      )}
    </div>
  );
});


export interface ExplorerProps extends BaseExplorerProps {
  basePath: string,
  pathsMap?: FilesystemEntryMap,
}
export const Explorer = React.memo(function Explorer(props: ExplorerProps) {
  const { basePath, loadAllRecursively = false, pathsMap = {} } = props;

  const pathsMapRef = React.useRef<FilesystemEntryMap>(pathsMap);
  const [updateBeacon, setUpdateBeacon] = React.useState({});
  const [root, setRoot] = React.useState<FilesystemEntry>();

  React.useEffect(() => {
    if (pathsMap) {
      return;
    }

    if (loadAllRecursively) {
      console.log('loading recursively');
      sendApiMessage(explorerApi.readDirRecursive, basePath);
    } else {
      console.log('loading normally');
      sendApiMessage(explorerApi.readDir, basePath);
    }
  }, [loadAllRecursively, basePath, pathsMap]);

  useApiMessage(explorerApi.root, (entry: FilesystemEntry) => {
    setRoot(entry);
  });
  useApiMessage(explorerApi.dirRecursive, (pathsMap: FilesystemEntryMap) => {
    console.log('dir rec', basePath, pathsMap);
    pathsMapRef.current = pathsMap;
    setUpdateBeacon({});
  });
  useApiMessage(explorerApi.dir, ({ dir, children }) => {
    pathsMapRef.current[dir] = children;
    setUpdateBeacon({});
  });
  const loadDir = React.useCallback((dir: string) => {
    setUpdateBeacon({});

    sendApiMessage(explorerApi.readDir, dir);
  }, []);

  const rootClassName = classnames(s.root, props.className || '', {
    [s.loading]: !root,
  });

  const basePathChilds = pathsMapRef.current[basePath] || [];

  const childsNodes = basePathChilds.map((child) => (
    <ExplorerDir
      key={child.path}
      {...props}
      childs={pathsMapRef.current[child.path]}
      childsCache={pathsMapRef.current}
      open={true}
      dir={child}
      loadDir={loadDir}
      updateBeacon={updateBeacon}
    />
  ));

  return (
    <div className={rootClassName}>
      {childsNodes}
    </div>
  );
});


/**
 * Explorer for FS roots a.k.a. drives
 */
export interface RootsExplorerProps extends BaseExplorerProps {
  basePath?: undefined,
  loadAllRecursively?: undefined,
}
export const RootsExplorer = React.memo(function RoosExplorer(props: RootsExplorerProps) {
  const childrensRef = React.useRef({});
  const [updateBeacon, setUpdateBeacon] = React.useState({});

  const [roots, setRoots] = React.useState<FilesystemEntry[]>([]);
  const [rootsLoaded, setRootsLoaded] = React.useState<boolean>(false);

  React.useEffect(() => {
    sendApiMessage(explorerApi.readRoots);
  }, []);

  useApiMessage(explorerApi.roots, (roots: FilesystemEntry[]) => {
    roots.forEach((root) => {
      childrensRef.current[root.path] = root.children || [];
    });

    setRootsLoaded(true);
    setRoots(roots);
  });

  useApiMessage(explorerApi.dir, ({ dir, children }) => {
    childrensRef.current[dir] = children;
    setUpdateBeacon({});
  });

  const loadDir = React.useCallback((dir: string) => {
    setUpdateBeacon({});

    sendApiMessage(explorerApi.readDir, dir);
  }, []);

  const dirs = roots.map((root, index) => (
    <ExplorerDir
      key={root.path}
      {...props}
      childs={childrensRef.current[root.path]}
      childsCache={childrensRef.current}
      open={index === 0}
      dir={root}
      loadDir={loadDir}
      updateBeacon={updateBeacon}
    />
  ));

  const rootClassName = classnames(s.root, props.className || '', {
    [s.loading]: !rootsLoaded,
  });

  return (
    <div className={rootClassName}>
      {dirs}
    </div>
  );
});
