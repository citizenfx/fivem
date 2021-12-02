import React from 'react';
import classnames from 'classnames';
import { observer } from "mobx-react-lite";
import { IFsEntry } from 'fxdk/project/common/project.types';
import { projectIcon } from 'fxdk/ui/icons';
import { Project } from 'fxdk/project/browser/state/project';
import { joinPath, renderExplorerItemIcon } from '../../explorer.utils';
import { noop, returnTrue } from 'fxdk/base/functional';
import { ExplorerRuntime } from '../../explorer.runtime';
import { useOpenFlag } from 'utils/hooks';
import s from './FsBrowser.module.scss';
import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';


export interface FsBrowserProps {
  className?: string,
  onSelectPath?(path: string): void,
  selectedPath?: string,
  selectableFilter?(entry: IFsEntry): boolean,
}

export const FsBrowser = observer(function FsBrowser(props: FsBrowserProps) {
  const {
    className,
    onSelectPath = noop,
    selectedPath = '',
    selectableFilter = returnTrue,
  } = props;

  const rootClassName = classnames(s.root, className);

  return (
    <div className={rootClassName}>
      <FsBrowserItem
        selectable
        expanded
        path={Project.path}
        entry={Project.fileSystem.fsEntry}
        onSelectPath={onSelectPath}
        selectedPath={selectedPath}
        selectableFilter={selectableFilter}
      />
    </div>
  );
});


type FsBrowserItemProps = Required<Omit<FsBrowserProps, 'className'>> & {
  path: string,
  entry: IFsEntry,
  selectable: boolean,
  expanded: boolean,
};
const FsBrowserItem = observer(function FsBrowserItem(props: FsBrowserItemProps) {
  const {
    path,
    entry,
    onSelectPath,
    selectedPath,
    selectableFilter,
  } = props;

  const [expanded, _open, _close, toggleExpanded] = useOpenFlag(props.expanded);

  const selectable = props.selectable && selectableFilter(entry);
  const isRootPath = path === Project.path;

  const handleClick = React.useCallback(() => {
    if (selectableFilter(entry)) {
      onSelectPath(path);
    }
  }, [path, entry, onSelectPath, selectableFilter]);

  const nameClassName = classnames(s.name, {
    [s.selected]: selectedPath === path,
    [s.disabled]: !selectable,
  });

  const icon = isRootPath
    ? <div className={s.icon}>{projectIcon}</div>
    : renderExplorerItemIcon(ExplorerRuntime.getItemDefaultIcon(path, expanded), s.icon);
  const label = isRootPath
    ? 'Project root'
    : entry.name;

  let children: React.ReactNode[] = [];
  if (expanded) {
    children = Object.values(entry.children).map((childEntry) => {
      const childEntryPath = joinPath(path, childEntry.name);

      return (
        <FsBrowserItem
          {...props}
          key={childEntryPath}
          path={childEntryPath}
          entry={childEntry}
          expanded={false}
          selectable={selectable}
        />
      );
    });
  }

  return (
    <div className={s.entry}>
      <div className={nameClassName} onClick={handleClick} onDoubleClick={toggleExpanded}>
        {icon}
        {label}
      </div>

      {!!children.length && (
        <div className={s.children}>
          {children}
        </div>
      )}
    </div>
  );
});
