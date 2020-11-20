import * as React from 'react';
import AnsiToHTMLConverter from 'ansi-to-html';
import { Switch, SwitchOption } from 'components/controls/Switch/Switch';
import { ServerUpdateChannel, serverUpdateChannels } from 'shared/api.types';
import { ProjectContext } from 'contexts/ProjectContext';
import { invariant } from 'utils/invariant';
import { ServerContext } from 'contexts/ServerContext';
import { projectApi } from 'shared/events';
import { sendApiMessage } from 'utils/api';
import { Modal } from 'components/Modal/Modal';
import { Button } from 'components/controls/Button/Button';
import s from './ServerConfig.module.scss';


const converter = new AnsiToHTMLConverter({
  newline: true,
  colors: {
    0: '#5F5F5F',
    1: '#D96468',
    2: '#A2D964',
    3: '#D9C964',
    4: '#64A2D9',
    5: '#9A64D9',
    6: '#64D9D5',
    7: '#989898',
    8: '#828282',
    9: '#D98F93',
    10: '#B8D98F',
    11: '#D9CF8F',
    12: '#8F99D9',
    13: '#B08FD9',
    14: '#8FD9D5',
    15: '#C5C5C5',
  }
});

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

  const updateChannel = project.manifest.serverUpdateChannel;

  const { serverOutput, checkForUpdates, sendServerCommand } = React.useContext(ServerContext);

  const [checkingForUpdates, setCheckingForUpdates] = React.useState(false);

  const handleCheckForUpdates = React.useCallback(() => {
    setCheckingForUpdates(true);
    checkForUpdates(updateChannel);
  }, [setCheckingForUpdates, checkForUpdates, updateChannel]);

  const handleUpdateChannelChange = React.useCallback((updateChannel: ServerUpdateChannel) => {
    sendApiMessage(projectApi.setServerUpdateChannel, updateChannel);
  }, []);

  const serverHtmlOutput = React.useMemo(() => {
    return converter.toHtml(serverOutput);
  }, [serverOutput]);

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

        <div
          className={s.output}
          dangerouslySetInnerHTML={{ __html: serverHtmlOutput }}
        >
        </div>

        <div className="modal-actions">
          <Button
            text="Check for updates"
            disabled={checkingForUpdates}
            onClick={handleCheckForUpdates}
          />
          <Button
            text="Show server GUI"
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
