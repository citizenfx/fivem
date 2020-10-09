import React from 'react';
import classnames from 'classnames';
import { BsPlayFill, BsStopFill } from 'react-icons/bs';
import { ServerContext } from '../../contexts/ServerContext';
import { rotatingRefreshIcon } from '../../constants/icons';
import { ServerStates } from '../../sdkApi/api.types';
import s from './Server.module.scss';

export const Server = React.memo(() => {
  const { serverState, startServer, stopServer } = React.useContext(ServerContext);

  const rootClassName = classnames(s.root, {
    [s.up]: serverState === ServerStates.up,
    [s.down]: serverState === ServerStates.down,
    [s.booting]: serverState === ServerStates.booting,
  });

  let icon;
  let title;

  switch (serverState) {
    case ServerStates.up: {
      icon = <BsStopFill />;
      title = 'Stop server';
      break;
    }

    case ServerStates.down: {
      icon = <BsPlayFill />;
      title = 'Start server';
      break;
    }

    case ServerStates.booting: {
      icon = rotatingRefreshIcon;
      title = 'Stop server';
      break;
    }
  }

  const handleClick = React.useCallback(() => {
    if (serverState === ServerStates.down) {
      startServer();
    }
    if (serverState === ServerStates.up) {
      stopServer();
    }
  }, [serverState, startServer, stopServer]);

  return (
    <div className={rootClassName} onClick={handleClick}>
      <div className={s.icon}>
        {icon}
      </div>
      <div className={s.title}>
        {title}
      </div>
    </div>
  );
});
