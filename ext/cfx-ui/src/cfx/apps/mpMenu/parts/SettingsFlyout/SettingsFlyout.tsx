import {
  Box,
  Flex,
  FlexRestricter,
  Page,
  Scrollable,
  NavList,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { useLocation } from 'react-router-dom';

import { useService } from 'cfx/base/servicesContainer';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { ISettingsUIService } from 'cfx/common/services/settings/settings.service';
import { ICategory } from 'cfx/common/services/settings/types';
import { Flyout } from 'cfx/ui/Flyout/Flyout';

import { SettingItem } from '../../../../common/parts/Settings/SettingItem/SettingItem';

function useCloseOnLocationChange(settingsUIService: ISettingsUIService) {
  const location = useLocation();
  React.useEffect(settingsUIService.close, [location]);
}

export const SettingsFlyout = observer(function SettingsFlyout() {
  const SettingsUIService = useService(ISettingsUIService);
  const eventHandler = useEventHandler();

  // Whenever location changes - close flyout
  useCloseOnLocationChange(SettingsUIService);

  const category = SettingsUIService.category!;

  const handleOnActivate = React.useCallback(
    (activateCategory: string) => {
      SettingsUIService.selectCategory(activateCategory);
      eventHandler({
        action: EventActionNames.SiteNavClick,
        properties: {
          text: activateCategory,
          link_url: '/',
          element_placement: ElementPlacements.Settings,
          position: 1,
        },
      });
    },
    [eventHandler, SettingsUIService],
  );

  return (
    <Flyout disableSoundEffects disabled={!SettingsUIService.visible} onClose={SettingsUIService.close} size="small">
      <Page>
        <Flex vertical fullWidth fullHeight gap="xlarge">
          <Flyout.Header onClose={SettingsUIService.close}>Settings</Flyout.Header>

          <FlexRestricter vertical>
            <Flex fullWidth fullHeight gap="xlarge">
              <Box noShrink width={50}>
                <NavList
                  items={SettingsUIService.navListItems}
                  onActivate={handleOnActivate}
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
  category: ICategory;
}
const Controls = observer(function Controls(props: ControlsProps) {
  const {
    category,
  } = props;

  const nodes: React.ReactNode[] = [];

  for (const [id, setting] of category.settings) {
    if (setting.visible && !setting.visible()) {
      continue;
    }

    nodes.push(<SettingItem key={id} setting={setting} />);
  }

  return (
    // eslint-disable-next-line react/jsx-no-useless-fragment
    <>{nodes}</>
  );
});
