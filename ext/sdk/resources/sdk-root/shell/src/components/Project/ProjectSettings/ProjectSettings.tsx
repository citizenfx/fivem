import * as React from 'react';
import { Switch, SwitchOption } from 'components/controls/Switch/Switch';
import { ServerUpdateChannel, serverUpdateChannels } from 'shared/api.types';
import { ProjectContext } from 'contexts/ProjectContext';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { Modal } from 'components/Modal/Modal';
import { Button } from 'components/controls/Button/Button';
import s from './ProjectSettings.module.scss';

const updateChannelOptions: SwitchOption[] = [
  {
    value: serverUpdateChannels.latest,
    label: 'Latest',
    description: 'Use latest available server artifact version, for those who feel edgy',
  },
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
];

export const ProjectSettings = React.memo(function ProjectSettings() {
  const { project, closeSettings } = React.useContext(ProjectContext);

  const updateChannel = project.manifest.serverUpdateChannel;
  const handleUpdateChannelChange = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(projectApi.setServerUpdateChannel, updateChannel);
  }, []);

  return (
    <Modal fullWidth onClose={closeSettings}>
      <div className={s.root}>
        <div className="modal-header">
          Project settings
        </div>

        <div className="modal-label">
          Server update channel:
        </div>
        <div className="modal-block">
          <Switch
            value={updateChannel}
            options={updateChannelOptions}
            onChange={handleUpdateChannelChange}
          />
        </div>

        <div className="modal-actions">
          <Button
            text="Close"
            onClick={closeSettings}
          />
        </div>
      </div>
    </Modal>
  );
});
