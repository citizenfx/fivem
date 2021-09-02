import * as React from 'react';
import { observer } from 'mobx-react-lite';
import { Switch, SwitchOption } from 'components/controls/Switch/Switch';
import { ServerUpdateChannel, serverUpdateChannels } from 'shared/api.types';
import { projectApi } from 'shared/api.events';
import { sendApiMessage } from 'utils/api';
import { Modal } from 'components/Modal/Modal';
import { Button } from 'components/controls/Button/Button';
import { useProjectDeployArtifactVar, useProjectSteamWebApiKeyVar, useProjectTebexSecretVar, useProjectUseTxAdminVar, useProjectUseVersioningVar } from 'utils/projectStorage';
import { Input } from 'components/controls/Input/Input';
import { ProjectState } from 'store/ProjectState';
import s from './ProjectSettings.module.scss';
import { ProjectSystemResources } from './ProjectSystemResources/ProjectSystemResources';
import { TabItem, TabSelector } from 'components/controls/TabSelector/TabSelector';
import { VscSymbolProperty } from 'react-icons/vsc';
import { enabledResourceIcon, projectBuildIcon } from 'constants/icons';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import { ProjectResourceSettings } from './ProjectResourceSettings/ProjectResourceSettings';

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

type KeysOf<T> = T[keyof T];

const projectSettingsTabs = {
  resources: 'resources',
  variables: 'variables',
  buildOptions: 'buildOptions',
};

type ProjectSettingsTab = KeysOf<typeof projectSettingsTabs>;

const settingsTabOptions: TabItem[] = [
  {
    label: 'Variables',
    value: projectSettingsTabs.variables,
    icon: <VscSymbolProperty />,
  },
  {
    label: 'Resources',
    value: projectSettingsTabs.resources,
    icon: enabledResourceIcon,
  },
  {
    label: 'Build Options',
    value: projectSettingsTabs.buildOptions,
    icon: projectBuildIcon,
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
  const [useVersioning, setUseVersioning] = useProjectUseVersioningVar(project);
  const [deployArtifact, setDeployArtifact] = useProjectDeployArtifactVar(project);
  const [useTxAdmin, setUseTxAdmin] = useProjectUseTxAdminVar(project);

  const [currentTab, setCurrentTab] = React.useState<ProjectSettingsTab>(projectSettingsTabs.variables);

  let tabNode: React.ReactNode;

  switch (currentTab) {
    case projectSettingsTabs.variables: {
      tabNode = (
        <div>
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

          <ProjectResourceSettings />
        </div>
      );
      break;
    }

    case projectSettingsTabs.resources: {
      tabNode = (
        <div>
          <ProjectSystemResources />
        </div>
      );
      break;
    }

    case projectSettingsTabs.buildOptions: {
      tabNode = (
        <div>
          <div className="modal-label">
            Deploy options:
          </div>
          <div className="modal-block">
            <Checkbox
              value={useVersioning}
              onChange={setUseVersioning}
              label="If possible, save previous build allowing build rollback"
            />
          </div>
          <div className="modal-block">
            <Checkbox
              value={deployArtifact}
              onChange={setDeployArtifact}
              label={`Include ${serverUpdateChannels[project.manifest.serverUpdateChannel]} server artifact`}
            />
          </div>
          {/*<div className="modal-block">
                <Checkbox
                  value={useTxAdmin}
                  onChange={setUseTxAdmin}
                  label="Use txAdmin to manage the server"
                />
              </div>*/}
        </div>
      );
      break;
    }
  }

  return (
    <Modal fullWidth fullHeight onClose={ProjectState.settingsUI.close}>
      <div className={s.root}>
        <div className={s.left}>
          <div className="modal-header">
            Project settings
          </div>
          <TabSelector
            className={s.tabs}
            value={currentTab}
            items={settingsTabOptions}
            onChange={setCurrentTab}
            vertical
          />
          <div className={s.actions}>
            <Button
              text="Close"
              onClick={ProjectState.settingsUI.close}
            />
          </div>
        </div>

        <div className={s.right}>
          {tabNode}
        </div>
      </div>
    </Modal>
  );
});
