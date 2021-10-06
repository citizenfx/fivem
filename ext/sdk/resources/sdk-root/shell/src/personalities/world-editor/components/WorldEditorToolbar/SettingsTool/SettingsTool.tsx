import React from 'react';
import { projectSettingsIcon } from 'constants/icons';
import { observer } from 'mobx-react-lite';
import { WETool } from 'personalities/world-editor/store/WEToolbarState';
import { BaseTool } from '../BaseTool/BaseTool';
import { div } from 'utils/styled';
import { WESettingsState } from 'personalities/world-editor/store/WESettingsState';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { useOpenFlag } from 'utils/hooks';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { GoKeyboard } from 'react-icons/go';
import { Hotkeys } from './Hotkeys/Hotkeys';
import s from './SettingsTool.module.scss';
import { IntroForceRecalculate } from 'fxdk/ui/Intro/Intro';
import { WEState } from 'personalities/world-editor/store/WEState';

const Root = div(s, 'root');
const Block = div(s, 'block');
const Label = div(s, 'label');
const Control = div(s, 'control');

export const SettingsTool = observer(function SettingsTool() {
  const [hotkeysOpen, openHotkeys, closeHotkeys] = useOpenFlag(false);

  return (
    <BaseTool
      tool={WETool.Settings}
      icon={projectSettingsIcon}
      label="Settings"

      toggleProps={{ 'data-intro-id': 'settings-panel' }}
      panelProps={{ 'data-intro-id': 'settings-panel' }}
    >
      <IntroForceRecalculate />

      <Root>
        <Block>
          <Label>
            Mouse sensetivity
          </Label>
          <Control>
            <input
              type="range"
              min={1}
              max={100}
              value={WESettingsState.settings.mouseSensetivity}
              onChange={({ target: { value } }) => WESettingsState.setMouseSensetivity(parseInt(value, 10))}
            />
          </Control>
        </Block>

        <Block>
          <Checkbox
            label="Show bounding box for selected entity"
            value={WESettingsState.settings.showSelectionBoundingBox}
            onChange={(value) => WESettingsState.setShowSelectionBoundingBox(value)}
          />
        </Block>

        <Block>
          <Checkbox
            label="Limit camera position above the ground"
            value={WESettingsState.settings.cameraAboveTheGround}
            onChange={(value) => WESettingsState.setCameraAboveTheGround(value)}
          />
        </Block>

        <Block>
          <Checkbox
            label="Spawn vehicle in play mode"
            value={WESettingsState.settings.playtestSpawnInVehicle}
            onChange={(value) => WESettingsState.setPlaytestSpawnInVehicle(value)}
          />
          <Input
            value={WESettingsState.settings.playtestVehicleName}
            onChange={(value) => WESettingsState.setPlaytestVehicleName(value)}
            placeholder="Vehicle model name"
          />
        </Block>

        <Block>
          <Button
            text="Hotkeys"
            icon={<GoKeyboard />}
            onClick={openHotkeys}
          />

          {hotkeysOpen && (
            <Hotkeys onClose={closeHotkeys} />
          )}
        </Block>

        <Block>
          <a href="javascript:void(0)" onClick={WEState.openIntro}>
            Show intro tour
          </a>
        </Block>
      </Root>
    </BaseTool>
  );
});
