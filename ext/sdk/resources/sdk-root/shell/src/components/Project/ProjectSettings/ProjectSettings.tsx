import * as React from 'react';
import { observer } from 'mobx-react-lite';
import { Switch, SwitchOption } from 'components/controls/Switch/Switch';
import { ServerUpdateChannel, serverUpdateChannels } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { Modal } from 'components/Modal/Modal';
import { Button } from 'components/controls/Button/Button';
import { useProjectSteamWebApiKeyVar, useProjectTebexSecretVar } from 'utils/projectStorage';
import { Input } from 'components/controls/Input/Input';
import { ProjectState } from 'store/ProjectState';
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

export const ProjectSettings = observer(function ProjectSettings() {
  const project = ProjectState.project;

  const updateChannel = project.manifest.serverUpdateChannel;
  const handleUpdateChannelChange = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(projectApi.setServerUpdateChannel, updateChannel);
  }, []);

  const [steamWebApiKey, setSteamWebApiKey] = useProjectSteamWebApiKeyVar(project);
  const [tebexSecret, setTebexSecret] = useProjectTebexSecretVar(project);

  return (
    <Modal fullWidth onClose={ProjectState.closeSettings}>
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

        <div className="modal-block modal-combine">
          <Input
            type="password"
            label="Steam API key:"
            value={steamWebApiKey}
            onChange={setSteamWebApiKey}
            description={<>Used only for build. If you want to use Steam authentication — <a href="https://steamcommunity.com/dev/apikey">get a key</a></>}
          />
          <Input
            type="password"
            label="Tebex secret:"
            value={tebexSecret}
            onChange={setTebexSecret}
            description={<a href="https://server.tebex.io/settings/servers">Get Tebex secret</a>}
          />
        </div>

        <div className="modal-actions">
          <Button
            text="Close"
            onClick={ProjectState.closeSettings}
          />
        </div>
      </div>
    </Modal>
  );
});
