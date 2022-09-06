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
      <Title title="Failed to load account data">
        <Button
          size="large"
          theme={buttonTheme}
          icon={<BsExclamationCircleFill style={{ color: 'rgba(var(--color-error))' }} />}
        />
      </Title>
    );
  }

  return (
    <Button
      theme="primary"
      size="large"
      text={$L('#BottomNav_LinkAccount')}
      onClick={AuthService.openUI}
    />
  );
});

function getUserAvatarTitle(account: IAccount): React.ReactNode {
  const titles: string[] = [account.username];

  // #TODOLOC
  if (account.isPremium) {
    titles.push('Premium');
  }
  if (account.isStaff) {
    titles.push('Staff member');
  }

  return (
    <Flex vertical centered gap="small">
      <span>
        {$L('#UserBar_AccountSettings')}
      </span>
      <strong>
        {titles.join(` ${Symbols.htmlDot} `)}
      </strong>
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
