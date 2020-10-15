import React from 'react';
import classnames from 'classnames';
import { BsExclamationTriangleFill, BsFillGearFill, BsPlayFill, BsStopFill } from 'react-icons/bs';
import { ServerContext } from '../../contexts/ServerContext';
import { rotatingRefreshIcon } from '../../constants/icons';
import { ServerStates, ServerUpdateStates } from '../../sdkApi/api.types';
import { ProjectContext } from '../../contexts/ProjectContext';
import s from './Server.module.scss';
import { useOpenFlag } from '../../utils/hooks';
import { ServerConfig } from './ServerConfig/ServerConfig';

export const Server = React.memo(() => {
  const { serverState, updateChannelsState, installationState, startServer, stopServer, installUpdate } = React.useContext(ServerContext);
  const { project } = React.useContext(ProjectContext);

  const [configuratorOpen, openConfigurator, closeConfigurator] = useOpenFlag(false);

  const updateChannelState = project
    ? updateChannelsState[project.manifest.serverUpdateChannel]
    : null;

  const installerState = project
    ? installationState[project.manifest.serverUpdateChannel]
    : null;

  const rootClassName = classnames(s.root, {
    [s.up]: serverState === ServerStates.up,
    [s.down]: serverState === ServerStates.down,
    [s.booting]: serverState === ServerStates.booting,
  });

  let icon;
  let title;
  let progress: React.ReactNode = null;

  if (updateChannelState === ServerUpdateStates.ready) {
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
  } else {
    switch (updateChannelState) {
      case ServerUpdateStates.updateRequired: {
        icon = <BsExclamationTriangleFill />;
        title = 'Install update';
        break;
      }

      case ServerUpdateStates.checking: {
        icon = rotatingRefreshIcon;
        title = 'Checking updates';
        break;
      }

      case ServerUpdateStates.updating: {
        icon = rotatingRefreshIcon;
        title = 'Updating';

        const percentage = ((installerState?.downloadedPercentage || 0) + (installerState?.unpackedPercentage || 0)) / 2;
        const width = `${percentage * 100}px`;

        progress = (
          <div
            className={s.progress}
            style={{ width }}
          />
        );
      }
    }
  }

  const handleClick = React.useCallback(() => {
    if (project && updateChannelState === ServerUpdateStates.updateRequired) {
      return installUpdate(project.manifest.serverUpdateChannel);
    }

    if (updateChannelState !== ServerUpdateStates.ready) {
      return;
    }

    if (serverState === ServerStates.down) {
      startServer();
    }
    if (serverState === ServerStates.up) {
      stopServer();
    }
  }, [serverState, updateChannelState, startServer, stopServer]);

  return (
    <div className={rootClassName}>
      {progress}

      <div className={s.button} onClick={handleClick}>
        <div className={s.icon}>
          {icon}
        </div>
        <div className={s.title}>
          {title}
        </div>
      </div>

      {updateChannelState === ServerUpdateStates.ready && (
        <>
          <div className={s.config} onClick={openConfigurator}>
            <BsFillGearFill />
          </div>
          {configuratorOpen && (
            <ServerConfig onClose={closeConfigurator} />
          )}
        </>
      )}
    </div>
  );
});
