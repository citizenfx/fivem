import React from 'react';
import classnames from 'classnames';
import { FilesystemEntry, FilesystemEntryMap } from 'shared/api.types';
import { explorerApi } from 'shared/api.events';
import { useApiMessage } from 'utils/hooks';
import { BaseExplorerProps } from './Explorer.types';
import { ExplorerDir } from './ExplorerDir';
import { readDir } from './Explorer.utils';
import s from './Explorer.module.scss';

export interface ExplorerProps extends BaseExplorerProps {
  baseEntry: FilesystemEntry,
  pathsMap?: FilesystemEntryMap,
}
export const Explorer = React.memo(function Explorer(props: ExplorerProps) {
  const { baseEntry, loadAllRecursively = false, pathsMap = {} } = props;

  const pathsMapRef = React.useRef<FilesystemEntryMap>(pathsMap);
  const [updateBeacon, setUpdateBeacon] = React.useState({});

  React.useEffect(() => {
    if (pathsMap) {
      return;
    }

    readDir(baseEntry.path, loadAllRecursively);
  }, [loadAllRecursively, baseEntry, pathsMap]);

  useApiMessage(explorerApi.dirRecursive, (pathsMap: FilesystemEntryMap) => {
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

  const rootClassName = classnames(s.root, props.className);

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
