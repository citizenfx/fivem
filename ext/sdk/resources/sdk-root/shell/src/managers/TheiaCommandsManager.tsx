import { ProjectContext } from 'contexts/ProjectContext';
import { ServerContext } from 'contexts/ServerContext';
import * as React from 'react';

export const TheiaCommandsManager = React.memo(function TheiaCommandsManager() {
  const { startServer } = React.useContext(ServerContext);
  const { build } = React.useContext(ProjectContext);

  const startServerRef = React.useRef(startServer);
  startServerRef.current = startServer;

  const buildRef = React.useRef(build);
  buildRef.current = build

  React.useEffect(() => {
    const handler = (e: MessageEvent) => {
      switch (e.data?.type) {
        case 'fxdk:startServer': {
          return startServerRef.current();
        }
        case 'fxdk:buildProject': {
          return buildRef.current();
        }
      }
    }

    window.addEventListener('message', handler);

    return () => window.removeEventListener('message', handler);
  }, [startServer, build]);

  return null;
});
