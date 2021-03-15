import React from 'react';
import classnames from 'classnames';
import { TheiaContext } from 'contexts/TheiaContext';
import { StateContext } from 'contexts/StateContext';
import { ProjectContext } from 'contexts/ProjectContext';
import { AppStates } from 'shared/api.types';
import s from './Theia.module.scss';


const address = {
  hostname: window.location.hostname,
  port: parseInt(window.location.port, 10) + 1,
};

export const TheiaPersonality = React.memo(function TheiaPersonality() {
  const { ref } = React.useContext(TheiaContext);
  const { state, toolbarOpen, toolbarWidth } = React.useContext(StateContext);
  const { project } = React.useContext(ProjectContext);

  const [showPersonality, setShowPersonality] = React.useState(false);
  const unveilTimerRef = React.useRef<any>(null);

  React.useEffect(() => () => {
    if (unveilTimerRef.current) {
      clearTimeout(unveilTimerRef.current);
    }
  }, []);

  React.useEffect(() => {
    if (state === AppStates.ready) {
      unveilTimerRef.current = setTimeout(() => {
        setShowPersonality(true);
      }, 500);
    }
  }, [state]);

  React.useEffect(() => {
    const handleMessage = (e) => {
      if (ref.current) {
        // Proxying all message from host to theia
        (ref.current as any).contentWindow.postMessage(e.data, '*');
      }
    }

    window.addEventListener('message', handleMessage);

    return () => window.removeEventListener('message', handleMessage);
  }, [ref]);

  const className = classnames(s.root, {
    [s.active]: showPersonality,
    [s.fullwidth]: !toolbarOpen,
  });

  if (!project) {
    return null;
  }

  const rootStyles: React.CSSProperties = {
    '--toolbar-width': `${toolbarWidth}px`,
  } as any;

  return (
    <iframe
      ref={ref}
      style={rootStyles}
      className={className}
      title="Theia personality"
      src={`http://${address.hostname}:${address.port}`}
      frameBorder="0"
      allowFullScreen={true}
    ></iframe>
  );
});
