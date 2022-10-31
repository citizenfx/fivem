import { $L } from "cfx/common/services/intl/l10n";
import { Button, ButtonTheme } from "cfx/ui/Button/Button";
import { observer } from "mobx-react-lite";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { BsExclamationCircleFill, BsFillStarFill } from "react-icons/bs";
import { Decorate } from "cfx/ui/Decorate/Decorate";
import { Badge } from "cfx/ui/Badge/Badge";
import { Symbols } from "cfx/ui/Symbols";
import { Title } from "cfx/ui/Title/Title";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Clickable } from "cfx/ui/Clickable/Clickable";
import { useService } from "cfx/base/servicesContainer";
import { ISettingsUIService } from "cfx/common/services/settings/settings.service";
import { useStreamerMode } from "cfx/apps/mpMenu/services/convars/convars.service";
import { useAccountService } from "cfx/common/services/account/account.service";
import { IAccount } from "cfx/common/services/account/types";
import { useAuthService } from "cfx/apps/mpMenu/services/auth/auth.service";
import { Icons } from "cfx/ui/Icons";
import { NavBarState } from "../NavBarState";

export const UserBar = observer(function UserBar() {
  const AuthService = useAuthService();
  const AccountService = useAccountService();
  const SettingsUIService = useService(ISettingsUIService);

  const buttonTheme: ButtonTheme = NavBarState.forceTransparentNav
    ? 'default'
    : 'default-blurred';

  const streamerMode = useStreamerMode();

  if (!AccountService.accountLoadComplete) {
    return (
      <Button
        size="large"
        theme={buttonTheme}
        icon={<Indicator />}
      />
    );
  }

  if (AccountService.account) {
    if (streamerMode) {
      return (
        <Button
          size="large"
          theme={buttonTheme}
          icon={Icons.accountLoaded}
          onClick={() => SettingsUIService.open('account')}
        />
      );
    }

    return (
      <Title title={getUserAvatarTitle(AccountService.account)}>
        <Decorate decorator={getUserAvatarDecorator(AccountService.account!)}>
          <Clickable onClick={() => SettingsUIService.open('account')}>
            <Avatar
              size="large"
              url={AccountService.account!.getAvatarUrl()}
            />
          </Clickable>
        </Decorate>
      </Title>
    );
  }

  if (AccountService.accountLoadError) {
    return (
      <Title title={$L('#UserNav_FailedToLoadAccountData')}>
        <Button
          size="large"
          theme={buttonTheme}
          icon={<BsExclamationCircleFill style={{ color: 'rgba(var(--color-error))' }} />}
        />
      </Title>
    );
  }

  const linkAccountButtonTheme = NavBarState.forceTransparentNav
    ? 'default'
    : 'primary';

  return (
    <Button
      theme={linkAccountButtonTheme}
      size="large"
      text={$L('#BottomNav_LinkAccount')}
      onClick={AuthService.openUI}
    />
  );
});

function getUserAvatarTitle(account: IAccount): React.ReactNode {
  const hasExtraTitle = account.isPremium || account.isStaff;

  return (
    <Flex vertical centered gap="small">
      <span>
        {$L('#UserBar_AccountSettings')}
      </span>

      {hasExtraTitle && (
        <strong>
          {account.isPremium && (
            $L('#UserNav_Title_Premium')
          )}

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
}
