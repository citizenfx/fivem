import * as React from 'react';
import Ansi from 'ansi-to-react';
import { ProjectContext } from '../../../contexts/ProjectContext';
import { ServerContext } from '../../../contexts/ServerContext';
import { ServerUpdateChannel, serverUpdateChannels } from '../../../sdkApi/api.types';
import { projectApi } from '../../../sdkApi/events';
import { sendApiMessage } from '../../../utils/api';
import { invariant } from '../../../utils/invariant';
import { Button } from '../../controls/Button/Button';
import { Switch, SwitchOption } from '../../controls/Switch/Switch';
import { Modal } from '../../Modal/Modal';
import s from './ServerConfig.module.scss';


const updateChannelOptions: SwitchOption[] = [
  {
    value: serverUpdateChannels.recommended,
    label: 'Recommended',
    description: 'Use server artifact of recommended version',
  },
  {
    value: serverUpdateChannels.optional,
    label: 'Optional',
    description: 'Use optional server artifact version',
  },
  {
    value: serverUpdateChannels.latest,
    label: 'Latest',
    description: 'Use latest available server artifact version, for those who feel edgy',
  },
];

export interface ServerConfigProps {
  onClose: () => void,
}

export const ServerConfig = React.memo(({ onClose }: ServerConfigProps) => {
  const { project } = React.useContext(ProjectContext);
  invariant(project, 'No project');

  const updateChannel = project.manifest.serverUpdateChannel;

  const { serverOutput, serverState, updateChannelsState, checkForUpdates } = React.useContext(ServerContext);

  const [checkingForUpdates, setCheckingForUpdates] = React.useState(false);

  const handleCheckForUpdates = React.useCallback(() => {
    setCheckingForUpdates(true);
    checkForUpdates(updateChannel);
  }, [setCheckingForUpdates, checkForUpdates, updateChannel]);

  const handleUpdateChannelChange = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(projectApi.setServerUpdateChannel, updateChannel);
  }, []);

  return (
    <Modal onClose={onClose}>
      <div className={s.root}>
        <div className="modal-header">
          Server configuration
        </div>

        <div className={s['update-channel']}>
          <div className={s.label}>
            Server update channel:
          </div>
          <Switch
            value={updateChannel}
            options={updateChannelOptions}
            onChange={handleUpdateChannelChange}
          />
        </div>

        <div
          className={s.output}
        >
          <Ansi>
            {serverOutput}
          </Ansi>
        </div>

        <div className="modal-actions">
          <Button
            text="Check for updates"
            disabled={checkingForUpdates}
            onClick={handleCheckForUpdates}
          />
          <Button
            text="Close"
            onClick={onClose}
          />
        </div>
      </div>
    </Modal>
  );
});
