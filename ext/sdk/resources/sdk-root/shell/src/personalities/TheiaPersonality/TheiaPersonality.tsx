import React from 'react';
import classnames from 'classnames';
import { AppStates } from 'shared/api.types';
import { ShellState } from 'store/ShellState';
import { ToolbarState } from 'store/ToolbarState';
import { observer } from 'mobx-react-lite';
import { ProjectState } from 'store/ProjectState';
import { TheiaState } from './TheiaState';
import s from './TheiaPersonality.module.scss';


const address = {
  hostname: window.location.hostname,
  port: parseInt(window.location.port, 10) + 1,
};

export const TheiaPersonality = observer(function TheiaPersonality() {
  const [showPersonality, setShowPersonality] = React.useState(false);
  const unveilTimerRef = React.useRef<any>(null);

  React.useEffect(() => () => {
    if (unveilTimerRef.current) {
      clearTimeout(unveilTimerRef.current);
    }
  }, []);

  React.useEffect(() => {
    if (ShellState.appState === AppStates.ready) {
      unveilTimerRef.current = setTimeout(() => {
        setShowPersonality(true);
      }, 500);
    }
  }, [ShellState.appState]);

  const className = classnames(s.root, {
    [s.active]: showPersonality,
    [s.fullwidth]: !ToolbarState.isOpen,
  });

  if (!ProjectState.hasProject) {
    return null;
  }

  const rootStyles: React.CSSProperties = {
    '--toolbar-width': `${ToolbarState.width}px`,
  } as any;

  return (
    <iframe
      ref={TheiaState.ref}
      style={rootStyles}
      className={className}
      title="Theia personality"
      src={`http://${address.hostname}:${address.port}`}
      frameBorder="0"
      allowFullScreen={true}
    ></iframe>
  );
});
