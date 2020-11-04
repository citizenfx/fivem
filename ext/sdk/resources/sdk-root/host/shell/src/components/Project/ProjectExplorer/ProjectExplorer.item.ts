import * as React from 'react';
import { FilesystemEntry, Project, ProjectResources } from 'sdkApi/api.types';

export type ProjectItemRenderer = (props: ProjectItemProps) => React.ReactNode;

export type ProjectItemFilter = (entry: FilesystemEntry) => boolean;

export interface ProjectItemProps {
  entry: FilesystemEntry,
  project: Project,
  projectResources: ProjectResources,
  pathsMap: {
    [path: string]: FilesystemEntry[],
  },
  itemRenderer: ProjectItemRenderer,
  creatorClassName: string,
}

export const getItemProps = <T extends ProjectItemProps>(props: T): ProjectItemProps => ({
  entry: props.entry,
  project: props.project,
  projectResources: props.projectResources,
  pathsMap: props.pathsMap,
  itemRenderer: props.itemRenderer,
  creatorClassName: props.creatorClassName,
});

export const renderChildren = (entry: FilesystemEntry, itemProps: ProjectItemProps, filter?: ProjectItemFilter): React.ReactNode => {
  let entryChildren = itemProps.pathsMap[entry.path] || [];

  if (filter) {
    entryChildren = entryChildren.filter(filter);
  }

  return entryChildren.map((childEntry) => itemProps.itemRenderer({
    ...getItemProps(itemProps),
    entry: childEntry,
  }));
};
