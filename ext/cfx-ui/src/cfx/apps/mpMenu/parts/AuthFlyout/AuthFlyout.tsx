import { Button } from "cfx/ui/Button/Button";
import { Icons } from "cfx/ui/Icons";
import { Separator } from "cfx/ui/Separator/Separator";
import { Pad } from "cfx/ui/Layout/Pad/Pad";
import { Flex } from "cfx/ui/Layout/Flex/Flex";
import { observer } from "mobx-react-lite";
import { Title } from "cfx/ui/Title/Title";
import { Flyout } from "cfx/ui/Flyout/Flyout";
import { Box } from "cfx/ui/Layout/Box/Box";
import { Center } from "cfx/ui/Layout/Center/Center";
import { useAuthService } from "cfx/apps/mpMenu/services/auth/auth.service";
import { AuthForm } from "cfx/common/parts/AuthForm/AuthForm";
import { renderedIf } from "cfx/utils/convenience";
import { $L } from "cfx/common/services/intl/l10n";
import s from './AuthFlyout.module.scss';

export const AuthFlyout = observer(renderedIf(
  () => useAuthService().UIOpen,
  observer(function AuthFlyout() {
    const AuthService = useAuthService();

    const handleModalClose = AuthService.authFormDisabled
      ? undefined
      : AuthService.dismiss;

    return (
      <Flyout
        size="small"
        onClose={handleModalClose}
        holderClassName={s.holder}
      >
        <Pad left size="large">
          <Flyout.Header onClose={handleModalClose}>
            {$L('#AuthUI_Header')}
          </Flyout.Header>
        </Pad>

        <Center vertical horizontal>
          <Box width="50%">
            <Pad size="large">
              <Flex vertical fullWidth fullHeight centered gap="xlarge">
                <AuthForm
                  onDone={AuthService.handleAuthFormDone}
                  onModeChange={AuthService.handleAuthFormModeChange}
                  onDisabledChange={AuthService.handleAuthFormDisabled}
                />

                {AuthService.showDismissAndIgnoreNextTime && (
                  <>
                    <Separator />

                    <Title title={(
                      <>
                        {$L('#AuthUI_CancelAndForget_Title')} {Icons.settings}
                      </>
                    )}>
                      <Button
                        text={$L('#AuthUI_CancelAndForget')}
                        theme="transparent"
                        disabled={AuthService.authFormDisabled}
                        onClick={AuthService.dismissIgnoreNextTime}
                      />
                    </Title>
                  </>
                )}
              </Flex>
            </Pad>
          </Box>
        </Center>
      </Flyout>
    );
  }),
));
