import * as React from 'react';
import { ProjectContext } from 'contexts/ProjectContext';
import { TheiaContext } from 'contexts/TheiaContext';
import { getFoldersForTheia } from 'utils/project';
import { logger } from 'utils/logger';

const log = logger('TheiaProjectManager');

export const TheiaProjectManager = React.memo(function TheiaProjectManager() {
  const { project } = React.useContext(ProjectContext);
  const { openProjectInTheia, theiaIsReady, sendTheiaMessage } = React.useContext(TheiaContext);

  const flags = React.useRef({
    reloadPending: false,
  });

  const lastProjectPathRef = React.useRef('');
  const lastFoldersStringRef = React.useRef('');

  // Handling project change so we open that in theia correctly
  React.useEffect(() => {
    if (!theiaIsReady && flags.current.reloadPending) {
      log('Theia is not ready and reload pending');
      flags.current.reloadPending = false;
    }

    if (project) {
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

        sendTheiaMessage({
          type: 'fxdk:forceReload',
        });

        return;
      }

      if (theiaIsReady && !flags.current.reloadPending) {
        log('Theia is ready and no reload is pending');

        const folders = getFoldersForTheia(project);
        const foldersString = JSON.stringify(folders);

        if (lastFoldersStringRef.current !== foldersString) {
          log('Opening project in theia');

          lastFoldersStringRef.current = foldersString;

          openProjectInTheia({
            name: project.manifest.name,
            path: project.path,
            folders,
          });
        }
      }
    }
  }, [project, theiaIsReady, openProjectInTheia, sendTheiaMessage]);

  return null;
});
