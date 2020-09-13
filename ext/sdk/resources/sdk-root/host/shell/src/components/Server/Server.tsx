import React from 'react';
import classnames from 'classnames';
import { BsPlayFill } from 'react-icons/bs';
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
      icon = <BsPlayFill />;
      title = 'Server is up';
      break;
    }

    case ServerStates.down: {
      icon = <BsPlayFill />;
      title = 'Start server';
      break;
    }

    case ServerStates.booting: {
      icon = rotatingRefreshIcon;
      title = 'Server is starting';
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
      {icon}
      {title}
    </div>
  );
});
