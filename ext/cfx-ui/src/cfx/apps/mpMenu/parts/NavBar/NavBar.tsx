import {
  Button,
  Icons,
  Box,
  Flex,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { useService } from 'cfx/base/servicesContainer';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { ISettingsUIService } from 'cfx/common/services/settings/settings.service';

import { Exitter } from './Exitter/Exitter';
import { HomeButton } from './HomeButton/HomeButton';
import { NavBarState } from './NavBarState';
import { UserBar } from './UserBar/UserBar';

export const NavBar = observer(function NavBar() {
  const SettingsUIService = useService(ISettingsUIService);
  const eventHandler = useEventHandler();

  const handleSettingsClick = React.useCallback(() => {
    SettingsUIService.open();
    eventHandler({
      action: EventActionNames.SiteNavClick,
      properties: {
        text: '#BottomNav_Settings',
        link_url: '/',
        element_placement: ElementPlacements.Nav,
        position: 0,
      },
    });
  }, [eventHandler, SettingsUIService]);

  React.useEffect(() => {
    NavBarState.setReady();

    return NavBarState.setNotReady;
  }, []);

  return (
    <Flex repell centered gap="large">
      {NavBarState.homeButtonVisible
        ? (
            <HomeButton />
          )
        : null}

      <Box grow ref={NavBarState.outletRef} />

      <UserBar />

      <Title title={$L('#BottomNav_Settings')}>
        <Button size="large" icon={Icons.settings} onClick={handleSettingsClick} />
      </Title>

      <Exitter />
    </Flex>
  );
});
