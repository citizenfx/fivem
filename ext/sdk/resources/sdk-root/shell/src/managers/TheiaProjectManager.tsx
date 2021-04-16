import React from 'react';
import { observer } from 'mobx-react-lite';
import { getFoldersForTheia } from 'utils/project';
import { logger } from 'utils/logger';
import { TheiaState } from 'store/TheiaState';
import { ProjectState } from 'store/ProjectState';

const log = logger('TheiaProjectManager');

export const TheiaProjectManager = observer(function TheiaProjectManager() {
  const flags = React.useRef({
    reloadPending: false,
  });

  const lastProjectPathRef = React.useRef('');
  const lastFoldersStringRef = React.useRef('');

  // Handling project change so we open that in theia correctly
  React.useEffect(() => {
    if (!TheiaState.isReady && flags.current.reloadPending) {
      log('Theia is not ready and reload pending');
      flags.current.reloadPending = false;
    }

    if (ProjectState.hasProject) {
      const project = ProjectState.project;

      // Initial project set
      if (lastProjectPathRef.current === '') {
        lastProjectPathRef.current = project.path;
      }

      // If path has changed - start reloading theia project sequence
      if (lastProjectPathRef.current !== project.path) {
        log('Project has changed, reloading');

        lastProjectPathRef.current = project.path;
        lastFoldersStringRef.current = '';

        flags.current.reloadPending = true;

        TheiaState.forceReload();

        return;
      }

      if (TheiaState.isReady && !flags.current.reloadPending) {
        log('Theia is ready and no reload is pending');

        const folders = getFoldersForTheia(project, ProjectState.pendingFolderDeletions);
        const foldersString = JSON.stringify(folders);

        if (lastFoldersStringRef.current !== foldersString) {
          log('Opening project in theia');

          lastFoldersStringRef.current = foldersString;

          TheiaState.openProject({
            name: project.manifest.name,
            path: project.path,
            folders,
          });
        }
      }
    } else {
      lastProjectPathRef.current = '';
      lastFoldersStringRef.current = '';
      TheiaState.setIsReady(false);
    }
  }, [ProjectState.hasProject, TheiaState.isReady, ProjectState.pendingFolderDeletions]);

  return null;
});
