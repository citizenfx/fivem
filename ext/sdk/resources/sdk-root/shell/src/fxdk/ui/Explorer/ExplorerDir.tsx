import * as React from 'react';
import classnames from 'classnames';
import { defaultVisibilityFilter } from './Explorer.filters';
import { ExplorerDirProps } from './Explorer.types';
import { getEntryIcon } from './Explorer.utils';
import s from './Explorer.module.scss';

const defaultSelectableFilter = () => true;
const noop = () => { };

export const ExplorerDir = React.memo(function ExplorerDir(props: ExplorerDirProps) {
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
