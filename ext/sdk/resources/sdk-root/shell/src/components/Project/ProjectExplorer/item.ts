import * as React from 'react';
import { FilesystemEntry, ProjectData } from 'shared/api.types';

export const entryCollator = new Intl.Collator(undefined, {
  usage: 'sort',
});

export const entriesSorter = (a: FilesystemEntry, b: FilesystemEntry) => {
  if (a.isDirectory && !b.isDirectory) {
    return -1;
  }

  if (!a.isDirectory && b.isDirectory) {
    return 1;
  }

  return entryCollator.compare(a.name, b.name);
};

export type ProjectItemRenderer = (props: ProjectItemProps) => React.ReactNode;

export type ProjectItemFilter = (entry: FilesystemEntry) => boolean;

export interface ProjectItemProps {
  entry: FilesystemEntry,
  project: ProjectData,
  pathsMap: {
    [path: string]: FilesystemEntry[],
  },
  itemRenderer: ProjectItemRenderer,
  creatorClassName: string,

  childrenCollapsed?: boolean | void,
}

export const getItemProps = <T extends ProjectItemProps>(props: T): ProjectItemProps => ({
  entry: props.entry,
  project: props.project,
  pathsMap: props.pathsMap,
  itemRenderer: props.itemRenderer,
  creatorClassName: props.creatorClassName,
  childrenCollapsed: props.childrenCollapsed,
});

export const renderChildren = (entry: FilesystemEntry, itemProps: ProjectItemProps, filter?: ProjectItemFilter): React.ReactNode => {
  let entryChildren = itemProps.pathsMap[entry.path] || [];

  if (filter) {
    entryChildren = entryChildren.filter(filter);
  }

  entryChildren.sort(entriesSorter);

  return entryChildren.map((childEntry) => itemProps.itemRenderer({
    ...getItemProps(itemProps),
    entry: childEntry,
  }));
};
