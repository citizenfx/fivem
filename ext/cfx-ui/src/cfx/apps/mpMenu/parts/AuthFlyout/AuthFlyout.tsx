import {
  Button,
  Icons,
  Box,
  Center,
  Flex,
  Pad,
  Separator,
  Title,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { useAuthService } from 'cfx/apps/mpMenu/services/auth/auth.service';
import { AuthForm } from 'cfx/common/parts/AuthForm/AuthForm';
import { $L } from 'cfx/common/services/intl/l10n';
import { Flyout } from 'cfx/ui/Flyout/Flyout';
import { renderedIf } from 'cfx/utils/convenience';

import s from './AuthFlyout.module.scss';

export const AuthFlyout = observer(
  renderedIf(
    () => useAuthService().UIOpen,
    observer(function AuthFlyout() {
      const AuthService = useAuthService();

      const handleModalClose = AuthService.authFormDisabled
        ? undefined
        : AuthService.dismiss;

      return (
        <Flyout size="small" onClose={handleModalClose} holderClassName={s.holder}>
          <Pad left size="large">
            <Flyout.Header onClose={handleModalClose}>{$L('#AuthUI_Header')}</Flyout.Header>
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

                      <Title
                        title={(
                          <>
                            {$L('#AuthUI_CancelAndForget_Title')} {Icons.settings}
                          </>
                        )}
                      >
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
  ),
);
