import {
  Avatar,
  Badge,
  Button,
  Decorate,
  Icons,
  Indicator,
  Interactive,
  Flex,
  Symbols,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { BsExclamationCircleFill, BsFillStarFill } from 'react-icons/bs';

import { useAuthService } from 'cfx/apps/mpMenu/services/auth/auth.service';
import { useStreamerMode } from 'cfx/apps/mpMenu/services/convars/convars.service';
import { useService } from 'cfx/base/servicesContainer';
import { useAccountService } from 'cfx/common/services/account/account.service';
import { IAccount } from 'cfx/common/services/account/types';
import { useEventHandler } from 'cfx/common/services/analytics/analytics.service';
import { EventActionNames, ElementPlacements } from 'cfx/common/services/analytics/types';
import { $L } from 'cfx/common/services/intl/l10n';
import { ISettingsUIService } from 'cfx/common/services/settings/settings.service';

import { NavBarState } from '../NavBarState';

type ButtonTheme = React.ComponentProps<typeof Button>['theme'];

export const UserBar = observer(function UserBar() {
  const AuthService = useAuthService();
  const AccountService = useAccountService();
  const SettingsUIService = useService(ISettingsUIService);
  const eventHandler = useEventHandler();

  const buttonTheme: ButtonTheme = NavBarState.forceTransparentNav
    ? 'default'
    : 'default-blurred';

  const streamerMode = useStreamerMode();

  const handleSettingsClick = React.useCallback(
    (text: string) => {
      SettingsUIService.open('account');
      eventHandler({
        action: EventActionNames.SiteNavClick,
        properties: {
          text,
          link_url: '/',
          element_placement: ElementPlacements.Nav,
          position: 0,
        },
      });
    },
    [eventHandler, SettingsUIService],
  );

  const handleAvatarClick = React.useCallback(() => {
    handleSettingsClick('UserBar_AccountSettings');
  }, [handleSettingsClick]);

  const handleStreamerClick = React.useCallback(() => {
    handleSettingsClick('Streamer');
  }, [handleSettingsClick]);

  const handleAuthClick = React.useCallback(() => {
    AuthService.openAuthUI();
    eventHandler({
      action: EventActionNames.SiteNavClick,
      properties: {
        text: '#BottomNav_LinkAccount',
        link_url: '/',
        element_placement: ElementPlacements.Nav,
        position: 0,
      },
    });
  }, [eventHandler, AuthService]);

  if (!AccountService.accountLoadComplete) {
    return (
      <Button size="large" theme={buttonTheme} icon={<Indicator />} />
    );
  }

  if (AccountService.account) {
    if (streamerMode) {
      return (
        <Button size="large" theme={buttonTheme} icon={Icons.accountLoaded} onClick={handleStreamerClick} />
      );
    }

    return (
      <Title title={getUserAvatarTitle(AccountService.account)}>
        <Decorate decorator={getUserAvatarDecorator(AccountService.account!)}>
          <Interactive onClick={handleAvatarClick}>
            <Avatar size="large" url={AccountService.account!.getAvatarUrl()} />
          </Interactive>
        </Decorate>
      </Title>
    );
  }

  if (AccountService.accountLoadError) {
    const title = (
      <>
        {$L('#UserNav_FailedToLoadAccountData')}
        <br />
        <br />
        <strong>{$L('#UserNav_FailedToLoadAccountData_2')}</strong>
      </>
    );

    return (
      <Title title={title}>
        <Button size="large" theme={buttonTheme} icon={<BsExclamationCircleFill />} />
      </Title>
    );
  }

  const linkAccountButtonTheme = NavBarState.forceTransparentNav
    ? 'default'
    : 'primary';

  return (
    <Button theme={linkAccountButtonTheme} size="large" text={$L('#BottomNav_LinkAccount')} onClick={handleAuthClick} />
  );
});

function getUserAvatarTitle(account: IAccount): React.ReactNode {
  const hasExtraTitle = account.isPremium || account.isStaff;

  return (
    <Flex vertical centered gap="small">
      <span>{$L('#UserBar_AccountSettings')}</span>

      {hasExtraTitle && (
        <strong>
          {account.isPremium && $L('#UserNav_Title_Premium')}

          {account.isStaff && (
            <>
              {Symbols.htmlDot}
              {$L('#UserNav_Title_Staff')}
            </>
          )}
        </strong>
      )}
    </Flex>
  );
}

function getUserAvatarDecorator(account: IAccount): React.ReactNode {
  if (account.isPremium || account.isStaff) {
    return (
      <Badge color="default">
        <BsFillStarFill />
      </Badge>
    );
  }

  return null;
}
