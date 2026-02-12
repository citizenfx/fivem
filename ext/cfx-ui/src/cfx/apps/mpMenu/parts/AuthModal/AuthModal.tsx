/* eslint-disable camelcase */
import {
  Button,
  Icons,
  Flex,
  Title,
  Modal,
  Text,
  Spacer,
  InfoPanel,
  Indicator,
  Interactive,
  Avatar,
  BrandIcon,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { useAuthService } from 'cfx/apps/mpMenu/services/auth/auth.service';
import { useAccountService } from 'cfx/common/services/account/account.service';
import { $L, $L_nl2br } from 'cfx/common/services/intl/l10n';
import { renderedIf } from 'cfx/utils/convenience';

import { IAuthFormState, useAuthFormState } from './AuthFormState';

import s from './AuthModal.module.scss';

export const AuthModal = observer(
  renderedIf(
    () => useAuthService().UIOpen,
    observer(function AuthModal() {
      const AuthService = useAuthService();

      const state = useAuthFormState();

      const {
        mode,
      } = state;

      const dismissModal = React.useCallback(() => {
        AuthService.dismissAuthUI(mode);
      }, [AuthService, mode]);

      const handleModalClose = state.disabled
        ? (null as unknown as undefined)
        : dismissModal;

      return (
        <Modal onClose={handleModalClose}>
          <Flex vertical fullWidth fullHeight centered gap="none" className={s.wrap}>
            <div className={s.composite}>
              {BrandIcon.composite}
            </div>

            <div className={s.customSplitter} />

            {state.isModeInitial && (
              <ExternalAuthInit state={state} />
            )}

            {(state.isModeExternalAuthInitialized || state.isModeExternalAuthProcessing) && (
              <ExternalAuthInProgress state={state} />
            )}

            {state.isModeAuthenticated && (
              <ExternalAuthSuccess state={state} />
            )}
          </Flex>
        </Modal>
      );
    }),
  ),
);

type ExternalAuthInitProps = {
  state: IAuthFormState;
};
const ExternalAuthInit = observer(function ExternalAuthInit(props: ExternalAuthInitProps) {
  const {
    state,
  } = props;

  const AuthService = useAuthService();

  const {
    mode,
  } = state;

  const handleSkipAndForget = React.useCallback(() => {
    AuthService.dismissAuthUIAndIgnoreNextTime(mode);
  }, [AuthService, mode]);

  return (
    <Flex vertical gap="large">
      <Flex vertical>
        <Text centered weight="bolder" size="xxlarge">
          {$L('#AuthForm_External_Lead')}
        </Text>

        <Text typographic centered opacity="50">
          {$L_nl2br('&AuthForm_External_Lead_Extra')}
        </Text>
      </Flex>

      <Spacer size="large" />

      <Flex centered vertical gap="large">
        <Button
          autofocus
          size="large"
          theme="primary"
          icon={BrandIcon.cfxre}
          text={$L('#AuthForm_External_Initialize2')}
          className={s.authbtn}
          onClick={state.beginExternalAuth}
        />

        {AuthService.showDismissAndIgnoreNextTime && (
          <Title
            title={(
              <>
                {$L('#AuthUI_CancelAndForget_Title')} {Icons.settings}
              </>
            )}
          >
            <Button
              size="large"
              text={$L('#AuthUI_CancelAndForget2')}
              onClick={handleSkipAndForget}
            />
          </Title>
        )}
      </Flex>
    </Flex>
  );
});

type ExternalAuthInProgressProps = {
  state: IAuthFormState;
};
const ExternalAuthInProgress = observer(function ExternalAuthIntermediate(props: ExternalAuthInProgressProps) {
  const {
    state,
  } = props;

  const AccountService = useAccountService();

  const showErrorMessageNode = !!state.errorMessage;
  const showAwaitingConfirmationNode = !showErrorMessageNode && !state.isModeExternalAuthProcessing;
  const showLoadingDataNode = !showErrorMessageNode && state.isModeExternalAuthProcessing;
  const showTroubleshootingNode = showErrorMessageNode || state.isModeExternalAuthInitialized;

  return (
    <>
      {showErrorMessageNode && (
        <InfoPanel type="error">
          {$L_nl2br(state.errorMessage)}
        </InfoPanel>
      )}

      {showAwaitingConfirmationNode && (
        <Flex centered vertical>
          <Indicator />

          <Text size="large" weight="bold">
            {$L('#AuthForm_External_Initialized_Title')}
          </Text>
          <Text opacity="50">
            {$L('#AuthForm_External_Initialized_Extra')}
          </Text>
        </Flex>
      )}

      {showLoadingDataNode && (
        <Flex centered vertical>
          <Indicator />

          <Text size="large" weight="bold">
            {$L('#AuthForm_External_Processing_Title')}
          </Text>
        </Flex>
      )}

      {showTroubleshootingNode && (
        <>
          <div className={s.customSplitter} />

          <Flex fullWidth vertical centered gap="large">
            <Flex vertical>
              <Text centered typographic opacity="50">
                {$L('#AuthForm_External_Troubleshooting_Question')}
              </Text>

              <Button
                size="large"
                text={$L('#AuthForm_TryAgain')}
                onClick={state.switchToModeInitial}
              />
            </Flex>

            <Text typographic centered uppercase opacity="50" weight="bold" size="large">
              {$L('#AuthForm_External_Troubleshooting_Or')}
            </Text>

            <Text typographic centered opacity="50">
              {$L_nl2br('&AuthForm_External_Troubleshooting_NoAccount_1')}
              <a href={AccountService.getSignUpURL()}>
                {$L('#AuthForm_External_Troubleshooting_NoAccount_2')}
              </a>
              {$L('#AuthForm_External_Troubleshooting_NoAccount_3')}
              <Interactive className={s.inlineButton} onClick={state.switchToModeInitial}>
                {$L('#AuthForm_External_Troubleshooting_NoAccount_4')}
              </Interactive>
            </Text>

            <Text typographic centered opacity="50">
              {$L('#AuthForm_External_Troubleshooting_LastResort')}
            </Text>
          </Flex>
        </>
      )}
    </>
  );
});

type ExternalAuthSuccessProps = {
  state: IAuthFormState;
};
const ExternalAuthSuccess = observer(function ExternalAuthSuccess(props: ExternalAuthSuccessProps) {
  const {
    state,
  } = props;

  const AccountService = useAccountService();

  return (
    <Flex centered vertical gap="xlarge">
      <Flex centered vertical gap="large">
        <Avatar size="large" url={AccountService.account?.getAvatarUrl() || ''} />

        <Flex gap="small">
          <Text size="xlarge" opacity="50">
            {$L('#AuthForm_Success_Welcome')}
          </Text>

          <Text size="xlarge">{AccountService.account?.username}</Text>
        </Flex>
      </Flex>

      <Spacer />

      <Button
        theme="primary"
        size="large"
        text={$L('#AuthForm_Success_Continue')}
        onClick={state.continueAuthenticated}
      />
    </Flex>
  );
});
