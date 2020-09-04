import React from 'react';
import classnames from 'classnames';
import { sendCommand } from '../utils/sendCommand';

import s from './Shell.module.scss';

const personalities = {
  theia: {
    hostname: window.location.hostname,
    port: parseInt(window.location.port, 10) + 1,
  },
};

const startSDKGame = () => sendCommand('localgame sdk-game');
const restartSDKGame = () => sendCommand('localrestart');

const giveRifle = () => sendCommand('weapon WEAPON_CARBINERIFLE');

export function Shell() {
  const timer = React.useRef<any>(null);

  const [toolbarOpen, setToolbarOpen] = React.useState(false);

  const timerCallback = React.useRef<any>(null);
  timerCallback.current = () => setToolbarOpen(false);

  const theiaRef = React.useRef<any>(null);

  React.useEffect(() => {
    const handleMessage = (e) => {
      if (theiaRef.current) {
        theiaRef.current.contentWindow.postMessage(e.data, '*');
      }
    }

    window.addEventListener('message', handleMessage);

    return () => window.removeEventListener('message', handleMessage);
  }, []);

  const toggleToolbar = React.useCallback(() => {
    setToolbarOpen(!toolbarOpen);
  }, [toolbarOpen]);

  const handleOpenDevtools = React.useCallback(() => {
    setToolbarOpen(false);
    window.openDevTools();
  }, [setToolbarOpen]);
  const handleStartSDKGame = React.useCallback(() => {
    setToolbarOpen(false);
    startSDKGame();
  }, [setToolbarOpen]);
  const handleRestartSDKGame = React.useCallback(() => {
    setToolbarOpen(false);
    restartSDKGame();
  }, [setToolbarOpen]);
  const handleGiveRifle = React.useCallback(() => {
    setToolbarOpen(false);
    giveRifle();
  }, [setToolbarOpen]);

  const handleMouseLeave = React.useCallback(() => {
    timer.current = setTimeout(timerCallback.current, 200);
  }, []);
  const handleMouseEnter = React.useCallback(() => {
    if (timer.current) {
      window.clearTimeout(timer.current);
    }
  }, []);

  React.useEffect(() => {
    return () => {
      if (timer.current) {
        window.clearTimeout(timer.current);
      }
    };
  }, []);

  const toolbarClasses = classnames(s.toolbar, {
    [s.active]: toolbarOpen,
  });

  return (
    <div className={s.root}>
      <div
        className={toolbarClasses}
        onMouseEnter={handleMouseEnter}
        onMouseLeave={handleMouseLeave}
      >
        <button className={s.trigger} onClick={toggleToolbar}>FxDK</button>

        <div className={s.dropdown}>
          <button onClick={handleOpenDevtools}>Open DevTools</button>
          <button onClick={handleStartSDKGame}>localgame sdk-game</button>
          <button onClick={handleRestartSDKGame}>localrestart sdk-game</button>
          <button onClick={handleGiveRifle}>give rifle</button>
        </div>
      </div>

      <iframe
        ref={theiaRef}
        title="Theia personality"
        src={`http://${personalities.theia.hostname}:${personalities.theia.port}`}
        className={s.theia}
        frameBorder="0"
        allowFullScreen={true}
      ></iframe>
    </div>
  );
}
