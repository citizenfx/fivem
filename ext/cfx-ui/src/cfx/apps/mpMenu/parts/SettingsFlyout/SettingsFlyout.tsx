import React from "react";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Flyout } from "cfx/ui/Flyout/Flyout";
import { Scrollable } from "cfx/ui/Layout/Scrollable/Scrollable";
import { observer } from "mobx-react-lite";
import { SettingItem } from "../../../../common/parts/Settings/SettingItem/SettingItem";
import { useLocation } from "react-router-dom";
import { NavList } from "cfx/ui/NavList/NavList";
import { ICategory } from "cfx/common/services/settings/types";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Page } from "cfx/ui/Layout/Page/Page";
import { FlexRestricter } from "cfx/ui/Layout/Flex/FlexRestricter";
import { useService } from "cfx/base/servicesContainer";
import { ISettingsUIService } from "cfx/common/services/settings/settings.service";

function useCloseOnLocationChange(settingsUIService: ISettingsUIService) {
  const location = useLocation();
  React.useEffect(settingsUIService.close, [location]);
}

export const SettingsFlyout = observer(function SettingsFlyout() {
  const SettingsUIService = useService(ISettingsUIService);

  // Whenever location changes - close flyout
  useCloseOnLocationChange(SettingsUIService);

  const category = SettingsUIService.category!;

  return (
    <Flyout
      disableSoundEffects
      disabled={!SettingsUIService.visible}
      onClose={SettingsUIService.close}
      size="small"
    >
      <Page>
        <Flex vertical fullWidth fullHeight gap="xlarge">
          <Flyout.Header onClose={SettingsUIService.close}>
            Settings
          </Flyout.Header>

          <FlexRestricter vertical>
            <Flex fullWidth fullHeight gap="xlarge">
              <Box noShrink width={50}>
                <NavList
                  items={SettingsUIService.navListItems}
                  onActivate={SettingsUIService.selectCategory}
                  activeItemId={SettingsUIService.categoryID}
                />
              </Box>

              <FlexRestricter>
                <Scrollable>
                  <Flex vertical gap="xlarge">
                    <Controls category={category} />
                  </Flex>
                </Scrollable>
              </FlexRestricter>
            </Flex>
          </FlexRestricter>
        </Flex>
      </Page>
    </Flyout>
  );
});

interface ControlsProps {
  category: ICategory,
}
const Controls = observer(function Controls(props: ControlsProps) {
  const { category } = props;

  const nodes: React.ReactNode[] = [];

  for (const [id, setting] of category.settings) {
    if (setting.visible && !setting.visible()) {
      continue;
    }

    nodes.push(
      <SettingItem key={id} setting={setting} />
    );
  }

  return (
    <>
      {nodes}
    </>
  );
});

