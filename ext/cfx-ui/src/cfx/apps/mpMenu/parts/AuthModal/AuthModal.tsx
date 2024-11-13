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
              <CompositeCfxLogo />
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

  const { mode } = state;

  const handleSkipAndForget = React.useCallback(() => {
    AuthService.dismissAuthUIAndIgnoreNextTime(mode);
  }, [AuthService, mode]);

  return (
    <Flex vertical gap="large">
      <Flex vertical>
        <Text centered family="secondary" weight="bolder" size="xxlarge">
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
          icon={CfxLogo()}
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

// TODO: Remove once we have that in the UI components library
function CompositeCfxLogo() {
  return (
    <svg
      viewBox="0 0 390 82"
      fill="none"
      xmlns="http://www.w3.org/2000/svg"
    >
      <path
        fill="currentColor"
        d="M113 0H117V81H113V0Z"
      />
      <path
        fill="currentColor"
        // eslint-disable-next-line @stylistic/max-len
        d="M9.30157 12L31.082 12.0516C35.6343 12.0172 39.0333 12.7374 41.2775 14.2109C44.0202 16.0172 45.3893 19.1234 45.3893 23.5308C45.3893 30.7877 41.9904 34.8996 35.1924 35.8651V35.967C38.19 36.7059 39.7055 39.1636 39.7389 43.34C39.7389 45.4792 39.6577 48.2195 39.4969 51.5595C39.4969 53.8006 39.975 55.7389 40.9342 57.3745H28.9986C28.4408 56.7762 28.1626 55.5581 28.1626 53.7174C28.3915 50.6413 28.5045 48.1191 28.5045 46.1492C28.5045 42.1707 26.5732 40.1808 22.7121 40.1808H14.9897L11.6212 56.274H0L9.30157 12ZM19.1551 21.2267L16.9239 31.9096H26.1849C31.3616 31.9096 33.9652 29.8867 34 25.8407C34 22.7647 31.905 21.2267 27.7149 21.2267H19.1551Z"
      />
      <path
        fill="currentColor"
        // eslint-disable-next-line @stylistic/max-len
        d="M53.4565 57.4247L51.134 42.5337L42.5163 57.3228L28.9015 57.2812L37.3178 66.2539L30.0968 82L45.5342 72.7217L57.0178 81.6442L54.8937 66.8551L68.1579 57.4247H53.4565ZM54.1505 76.6829L45.6835 70.1133L34.9432 76.5294L39.891 65.8465L33.9073 59.4806H43.7536L49.9387 48.8063L51.6266 59.5825H61.3715L52.5626 65.7963L54.1505 76.6829Z"
      />
      <g clipPath="url(#clip0_1099_177)">
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M227.499 9H215.02L216.054 21.8535H204.645L205.679 9H193.306L163 73H200.491L201.721 57.7933H218.96L220.19 73H257.788L227.481 9H227.499ZM202.594 47.2039L204.217 27.2017H216.5L218.122 47.2039H202.594Z"
        />
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M300.77 69C293.5 69 287.548 66.4685 282.93 61.4056C278.312 56.3256 276.002 49.5351 276.002 41C276.002 32.4649 278.312 25.6744 282.93 20.5944C287.548 15.5315 293.483 13 300.77 13C306.397 13 311.272 14.5394 315.411 17.584C319.55 20.6286 322.27 25.2639 323.57 31.4386H317.173C316.01 27.0257 313.991 23.81 311.084 21.8088C308.176 19.8076 304.738 18.7984 300.77 18.7984C295.296 18.7984 290.918 20.7654 287.599 24.6995C284.281 28.6335 282.639 34.0727 282.639 41C282.639 47.9273 284.298 53.3494 287.599 57.3005C290.918 61.2346 295.296 63.2016 300.77 63.2016C304.892 63.2016 308.552 61.9359 311.751 59.4044C314.966 56.8729 316.899 53.1784 317.549 48.3036H323.878C323.228 54.923 320.696 60.0373 316.283 63.6121C311.87 67.204 306.705 69 300.787 69H300.77Z"
        />
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M346.37 19.3286C342.915 19.3286 341.17 21.1588 341.17 24.8192V28.9585H349.449V34.2266H341.17V67.7856H335.15V34.2266H328.753V28.9585H335.15V24.9731C335.15 17.7037 338.759 14.0605 345.994 14.0605C347.345 14.0605 348.628 14.1118 349.825 14.2144V19.3286H346.37Z"
        />
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M388.67 28.9585L374.593 47.9273L389.422 67.8027H382.341L380.237 64.8607C377.021 60.2938 373.994 56.0861 371.137 52.2205C366.93 57.8992 363.834 62.124 361.884 64.9462L359.849 67.8027H352.853L367.682 47.9273L353.605 28.9585H360.687L363.697 33.0977C366.656 37.1173 369.17 40.6237 371.223 43.6341C373.789 40.025 376.286 36.5015 378.749 33.0977L381.691 28.9585H388.687H388.67Z"
        />
      </g>
      <defs>
        <clipPath id="clip0_1099_177">
          <rect width="224.003" height="74" fill="currentColor" transform="translate(165 4)" />
        </clipPath>
      </defs>
    </svg>
  );
}

// TODO: Remove once we have that in the UI components library
function CfxLogo() {
  return (
    <svg
      viewBox="0 0 224 74"
      fill="none"
      xmlns="http://www.w3.org/2000/svg"
    >
      <g clipPath="url(#clip0_1099_125)">
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M62.4992 5H50.0201L51.054 17.8535H39.6446L40.6785 5H28.3064L-2 69H35.4908L36.7209 53.7933H53.9599L55.19 69H92.7877L62.4813 5H62.4992ZM37.5944 43.2039L39.2167 23.2017H51.4997L53.122 43.2039H37.5944Z"
        />
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M135.77 65C128.5 65 122.548 62.4685 117.93 57.4056C113.312 52.3256 111.002 45.5351 111.002 37C111.002 28.4649 113.312 21.6744 117.93 16.5944C122.548 11.5315 128.483 9 135.77 9C141.397 9 146.272 10.5394 150.411 13.584C154.55 16.6286 157.27 21.2639 158.57 27.4386H152.173C151.01 23.0257 148.991 19.81 146.084 17.8088C143.176 15.8076 139.738 14.7984 135.77 14.7984C130.296 14.7984 125.918 16.7654 122.599 20.6995C119.281 24.6335 117.639 30.0727 117.639 37C117.639 43.9273 119.298 49.3494 122.599 53.3005C125.918 57.2346 130.296 59.2016 135.77 59.2016C139.892 59.2016 143.552 57.9359 146.751 55.4044C149.966 52.8729 151.899 49.1784 152.549 44.3036H158.878C158.228 50.923 155.696 56.0373 151.283 59.6121C146.87 63.204 141.705 65 135.787 65H135.77Z"
        />
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M181.37 15.3286C177.915 15.3286 176.17 17.1588 176.17 20.8192V24.9585H184.449V30.2266H176.17V63.7856H170.15V30.2266H163.753V24.9585H170.15V20.9731C170.15 13.7037 173.759 10.0605 180.994 10.0605C182.345 10.0605 183.628 10.1118 184.825 10.2144V15.3286H181.37Z"
        />
        <path
          fill="currentColor"
          // eslint-disable-next-line @stylistic/max-len
          d="M223.67 24.9585L209.593 43.9273L224.422 63.8027H217.341L215.237 60.8607C212.021 56.2938 208.994 52.0861 206.137 48.2205C201.93 53.8992 198.834 58.124 196.884 60.9462L194.849 63.8027H187.853L202.682 43.9273L188.605 24.9585H195.687L198.697 29.0977C201.656 33.1173 204.17 36.6237 206.223 39.6341C208.789 36.025 211.286 32.5015 213.749 29.0977L216.691 24.9585H223.687H223.67Z"
        />
      </g>
      <defs>
        <clipPath id="clip0_1099_125">
          <rect width="224.003" height="74" fill="currentColor" />
        </clipPath>
      </defs>
    </svg>
  );
}
