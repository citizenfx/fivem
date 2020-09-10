import React from 'react';
import classnames from 'classnames';

import { sendCommand } from '../../utils/sendCommand';
import { States, StateContext } from '../State';

import s from './Toolbar.module.scss';


const startSDKGame = () => sendCommand('localgame sdk-game');
const restartSDKGame = () => sendCommand('localrestart');

const giveRifle = () => sendCommand('weapon WEAPON_CARBINERIFLE');

export const Toolbar = React.memo(({ sendTheiaMessage }: any) => {
  const { state } = React.useContext(StateContext);
  const timer = React.useRef<any>(null);
  const timerCallback = React.useRef<any>(null);

  const [toolbarOpen, setToolbarOpen] = React.useState(false);

  timerCallback.current = () => setToolbarOpen(false);

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


  const handleAddFolder = React.useCallback(() => {
    setToolbarOpen(false);
    sendTheiaMessage({ type: 'fxdk:addFolder', data: 'file:///C:/dev/fivem/fivem.net' });
  }, [setToolbarOpen]);
  const handleRemoveFolder = React.useCallback(() => {
    setToolbarOpen(false);
    sendTheiaMessage({ type: 'fxdk:removeFolder', data: 'file:///C:/dev/fivem/fivem.net' });
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
    [s.visible]: state === States.ready,
    [s.active]: toolbarOpen,
  });

  return (
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
        <button onClick={handleAddFolder}>add folder</button>
        <button onClick={handleRemoveFolder}>remove folder</button>
      </div>
    </div>
  );
});
