import * as React from 'react';
import { Switch, SwitchOption } from 'components/controls/Switch/Switch';
import { ServerStates, ServerUpdateChannel, serverUpdateChannels } from 'shared/api.types';
import { ProjectContext } from 'contexts/ProjectContext';
import { invariant } from 'utils/invariant';
import { ServerContext } from 'contexts/ServerContext';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { Modal } from 'components/Modal/Modal';
import { Button } from 'components/controls/Button/Button';
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

export const ServerConfig = React.memo(function ServerConfig({ onClose }: ServerConfigProps) {
  const { project } = React.useContext(ProjectContext);
  invariant(project, 'No project');

  const { serverState } = React.useContext(ServerContext);

  const updateChannel = project.manifest.serverUpdateChannel;

  const { sendServerCommand } = React.useContext(ServerContext);

  const handleUpdateChannelChange = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(projectApi.setServerUpdateChannel, updateChannel);
  }, []);

  const canShowServerGui = serverState !== ServerStates.down;

  return (
    <Modal fullWidth onClose={onClose}>
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

        <div className="modal-actions">
          <Button
            text="Show server GUI"
            disabled={!canShowServerGui}
            onClick={() => sendServerCommand('svgui')}
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
