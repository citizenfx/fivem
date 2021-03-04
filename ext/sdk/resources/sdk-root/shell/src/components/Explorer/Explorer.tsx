import React from 'react';
import classnames from 'classnames';
import { FilesystemEntry, FilesystemEntryMap } from 'shared/api.types';
import { explorerApi } from 'shared/api.events';
import { useApiMessage } from 'utils/hooks';
import { BaseExplorerProps } from './Explorer.types';
import { ExplorerDir } from './ExplorerDir';
import { readDir, readDrives } from './Explorer.utils';
import s from './Explorer.module.scss';

export interface ExplorerProps extends BaseExplorerProps {
  baseEntry: FilesystemEntry,
  pathsMap?: FilesystemEntryMap,
}
export const Explorer = React.memo(function Explorer(props: ExplorerProps) {
  const { baseEntry, loadAllRecursively = false, pathsMap = {} } = props;

  const pathsMapRef = React.useRef<FilesystemEntryMap>(pathsMap);
  const [updateBeacon, setUpdateBeacon] = React.useState({});
  const [root, setRoot] = React.useState<FilesystemEntry>();

  React.useEffect(() => {
    if (pathsMap) {
      return;
    }

    readDir(baseEntry.path, loadAllRecursively);
  }, [loadAllRecursively, baseEntry, pathsMap]);

  useApiMessage(explorerApi.root, (entry: FilesystemEntry) => {
    setRoot(entry);
  });
  useApiMessage(explorerApi.dirRecursive, (pathsMap: FilesystemEntryMap) => {
    console.log('dir rec', baseEntry, pathsMap);
    pathsMapRef.current = pathsMap;
    setUpdateBeacon({});
  });
  useApiMessage(explorerApi.dir, ({ dir, children }) => {
    pathsMapRef.current[dir] = children;
    setUpdateBeacon({});
  });
  const loadDir = React.useCallback((dir: string) => {
    setUpdateBeacon({});

    readDir(dir);
  }, []);

  const rootClassName = classnames(s.root, props.className || '', {
    [s.loading]: !root,
  });

  return (
    <div className={rootClassName}>
      <ExplorerDir
        {...props}
        childs={pathsMapRef.current[baseEntry.path]}
        childsCache={pathsMapRef.current}
        open={true}
        dir={baseEntry}
        loadDir={loadDir}
        updateBeacon={updateBeacon}
      />
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
    readDrives();
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
    readDir(dir);
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
