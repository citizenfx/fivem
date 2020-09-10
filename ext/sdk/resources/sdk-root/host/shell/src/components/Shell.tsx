import React from 'react';
import classnames from 'classnames';

import { Toolbar } from './Toolbar/Toolbar';
import { StateContext, States } from './State';
import { Preparing } from './Preparing/Preparing';

import s from './Shell.module.scss';


const personalities = {
  theia: {
    hostname: window.location.hostname,
    port: parseInt(window.location.port, 10) + 1,
  },
};

const boot = () => {
  window.fetch('http://127.0.0.1:35419/ready');
};

export function Shell() {
  const [showPersonality, setShowPersonality] = React.useState(false);
  const { state } = React.useContext(StateContext);
  const theiaRef = React.useRef<any>(null);
  const unveilTimerRef = React.useRef<any>(null);

  React.useEffect(() => {
    boot();

    const handleMessage = (e) => {
      if (theiaRef.current) {
        theiaRef.current.contentWindow.postMessage(e.data, '*');
      }
    }

    window.addEventListener('message', handleMessage);

    return () => window.removeEventListener('message', handleMessage);
  }, []);

  const sendTheiaMessage = React.useCallback((msg) => {
    if (theiaRef.current) {
      theiaRef.current.contentWindow.postMessage(msg, '*');
    }
  }, []);

  React.useEffect(() => () => {
    if (unveilTimerRef.current) {
      clearTimeout(unveilTimerRef.current);
    }
  }, []);

  React.useEffect(() => {
    if (state === States.ready) {
      unveilTimerRef.current = setTimeout(() => {
        setShowPersonality(true);
      }, 500);
    }
  }, [state]);

  const theiaClassName = classnames(s.theia, {
    [s.active]: showPersonality,
  });

  return (
    <div className={s.root}>
      <Toolbar sendTheiaMessage={sendTheiaMessage} />

      {!showPersonality && (
        <Preparing />
      )}

      <iframe
          ref={theiaRef}
          title="Theia personality"
          src={`http://${personalities.theia.hostname}:${personalities.theia.port}`}
          className={theiaClassName}
          frameBorder="0"
          allowFullScreen={true}
        ></iframe>
    </div>
  );
}
