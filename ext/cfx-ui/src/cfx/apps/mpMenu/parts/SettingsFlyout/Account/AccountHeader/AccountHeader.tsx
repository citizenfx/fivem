import { useAuthService } from "cfx/apps/mpMenu/services/auth/auth.service";
import { KnownConvars, useConvarService, useStreamerMode } from "cfx/apps/mpMenu/services/convars/convars.service";
import { useAccountService } from "cfx/common/services/account/account.service";
import { $L } from "cfx/common/services/intl/l10n";
import { Avatar } from "cfx/ui/Avatar/Avatar";
import { Button } from "cfx/ui/Button/Button";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { Text } from "cfx/ui/Text/Text";
import { observer } from "mobx-react-lite";

export const AccountHeader = observer(function AccountHeader() {
  const AuthService = useAuthService();
  const AccountService = useAccountService();

  if (!AccountService.account) {
    return (
      <Button
        size="large"
        text={$L('#BottomNav_LinkAccount')}
        theme="primary"
        onClick={AuthService.openUI}
      />
    );
  }

  return (
    <AccountHeaderSignedIn />
  );
});

const AccountHeaderSignedIn = observer(function AccountHeaderSignedIn() {
  const ConvarService = useConvarService();
  const AccountService = useAccountService();

  const node = useStreamerMode()
    ? (
      <span>
        {'<HIDDEN>'}
      </span>
    )
    : (
      <Flex centered gap="large">
        <Avatar
          size="large"
          url={AccountService.account?.getAvatarUrl() || ''}
        />

        <Text size="large">
          {AccountService.account?.username}
        </Text>
      </Flex>
    );

  return (
    <Flex centered="axis" repell>
      {node}

      <Button
        size="small"
        text="Sign Out"
        theme="transparent"
        onClick={AccountService.signout}
      />
    </Flex>
  );
});
