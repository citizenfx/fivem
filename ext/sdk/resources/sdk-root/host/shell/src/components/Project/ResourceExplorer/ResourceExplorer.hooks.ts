import * as React from 'react';
import { ResourceExplorerContext } from './ResourceExplorer.context';

export interface UseExpandedPathHook {
  expanded: boolean,
  toggleExpanded: () => void,
  forceExpanded: () => void,
  forceCollapsed: () => void,
}

export const useExpandablePath = (path: string, expandedByDefault: boolean = true): UseExpandedPathHook => {
  const { pathsState, setPathState } = React.useContext(ResourceExplorerContext);

  const expanded = path in pathsState
    ? pathsState[path]
    : expandedByDefault;

  const toggleExpanded = React.useCallback(() => {
    setPathState(path, !expanded);
  }, [path, expanded, setPathState]);

  const forceExpanded = React.useCallback(() => {
    setPathState(path, true);
  }, [path, setPathState]);

  const forceCollapsed = React.useCallback(() => {
    setPathState(path, false);
  }, [path, setPathState]);

  return {
    expanded,
    toggleExpanded,
    forceExpanded,
    forceCollapsed,
  };
};
