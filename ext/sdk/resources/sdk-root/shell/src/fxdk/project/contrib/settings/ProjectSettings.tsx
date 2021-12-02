import React from 'react';
import { observer } from 'mobx-react-lite';
import { Switch, SwitchOption } from 'fxdk/ui/controls/Switch/Switch';
import { serverUpdateChannels } from 'shared/api.types';
import { Modal, ModalActions, ModalHeader } from 'fxdk/ui/Modal/Modal';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { ProjectSystemResources } from './ProjectSystemResources/ProjectSystemResources';
import { TabItem, TabSelector } from 'fxdk/ui/controls/TabSelector/TabSelector';
import { VscSymbolProperty } from 'react-icons/vsc';
import { closedResourceIcon, projectBuildIcon } from 'fxdk/ui/icons';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { ProjectResourceSettings } from './ProjectResourceSettings/ProjectResourceSettings';
import { SettingsState } from './SettingsState';
import { Project } from 'fxdk/project/browser/state/project';
import s from './ProjectSettings.module.scss';
import { SplitHorizontal } from 'fxdk/ui/Modal/SplitHorizontalLayout';

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
    icon: closedResourceIcon,
  },
  {
    label: 'Build Options',
    value: projectSettingsTabs.buildOptions,
    icon: projectBuildIcon,
  },
];

export const ProjectSettings = observer(function ProjectSettings() {
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
              value={Project.manifest.serverUpdateChannel}
              options={updateChannelOptions}
              onChange={Project.setServerUpdateChannel}
            />
          </div>

          <div className="modal-block modal-combine">
            <Input
              type="password"
              label="Steam API key:"
              value={Project.localStorage.steamWebApiKey}
              onChange={(value) => Project.localStorage.steamWebApiKey = value}
              description={<>Used only for build. If you want to use Steam authentication — <a href="https://steamcommunity.com/dev/apikey">get a key</a></>}
            />
            <Input
              type="password"
              label="Tebex secret:"
              value={Project.localStorage.tebexSecret}
              onChange={(value) => Project.localStorage.tebexSecret = value}
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
              value={Project.localStorage.buildUseVersioning}
              onChange={(value) => Project.localStorage.buildUseVersioning = value}
              label="If possible, save previous build allowing build rollback"
            />
          </div>
          <div className="modal-block">
            <Checkbox
              value={Project.localStorage.buildUseArtifact}
              onChange={(value) => Project.localStorage.buildUseArtifact = value}
              label={`Include ${serverUpdateChannels[Project.manifest.serverUpdateChannel]} server artifact`}
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
    <Modal fullWidth fullHeight onClose={SettingsState.close}>
      <SplitHorizontal.Layout>
        <SplitHorizontal.Left>
          <ModalHeader>
            Project Settings
          </ModalHeader>

          <TabSelector
            className={s.tabs}
            value={currentTab}
            items={settingsTabOptions}
            onChange={setCurrentTab}
            vertical
          />

          <ModalActions>
            <Button
              text="Close"
              onClick={SettingsState.close}
            />
          </ModalActions>
        </SplitHorizontal.Left>

        <SplitHorizontal.Right>
          {tabNode}
        </SplitHorizontal.Right>
      </SplitHorizontal.Layout>
    </Modal>
  );
});
