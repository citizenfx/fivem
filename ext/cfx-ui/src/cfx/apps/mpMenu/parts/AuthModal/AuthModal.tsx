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
    <svg viewBox="0 0 475 82" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path d="M113 0H117V81H113V0Z" fill="currentColor" />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M9.30157 12L31.082 12.0516C35.6343 12.0172 39.0333 12.7374 41.2775 14.2109C44.0202 16.0172 45.3893 19.1234 45.3893 23.5308C45.3893 30.7877 41.9904 34.8996 35.1924 35.8651V35.967C38.19 36.7059 39.7055 39.1636 39.7389 43.34C39.7389 45.4792 39.6577 48.2195 39.4969 51.5595C39.4969 53.8006 39.975 55.7389 40.9342 57.3745H28.9986C28.4408 56.7762 28.1626 55.5581 28.1626 53.7174C28.3915 50.6413 28.5045 48.1191 28.5045 46.1492C28.5045 42.1707 26.5732 40.1808 22.7121 40.1808H14.9897L11.6212 56.274H0L9.30157 12ZM19.1551 21.2267L16.9239 31.9096H26.1849C31.3616 31.9096 33.9652 29.8867 34 25.8407C34 22.7647 31.905 21.2267 27.7149 21.2267H19.1551Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M53.4565 57.4247L51.134 42.5337L42.5163 57.3228L28.9015 57.2812L37.3178 66.2539L30.0968 82L45.5342 72.7217L57.0178 81.6442L54.8937 66.8551L68.1579 57.4247H53.4565ZM54.1505 76.6829L45.6835 70.1133L34.9432 76.5294L39.891 65.8465L33.9073 59.4806H43.7536L49.9387 48.8063L51.6266 59.5825H61.3715L52.5626 65.7963L54.1505 76.6829Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M229.499 9H217.02L218.054 21.8535H206.645L207.679 9H195.306L165 73H202.491L203.721 57.7933H220.96L222.19 73H259.788L229.481 9H229.499ZM204.594 47.2039L206.217 27.2017H218.5L220.122 47.2039H204.594Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M304.767 69C297.498 69 291.546 66.4685 286.927 61.4056C282.309 56.3256 280 49.5351 280 41C280 32.4649 282.309 25.6744 286.927 20.5944C291.546 15.5315 297.481 13 304.767 13C310.395 13 315.269 14.5394 319.409 17.584C323.548 20.6286 326.268 25.2639 327.567 31.4386H321.17C320.007 27.0257 317.989 23.81 315.081 21.8088C312.173 19.8076 308.735 18.7984 304.767 18.7984C299.294 18.7984 294.915 20.7654 291.597 24.6995C288.279 28.6335 286.637 34.0727 286.637 41C286.637 47.9273 288.296 53.3494 291.597 57.3005C294.915 61.2346 299.294 63.2016 304.767 63.2016C308.889 63.2016 312.55 61.9359 315.748 59.4044C318.964 56.8729 320.897 53.1784 321.547 48.3036H327.875C327.225 54.923 324.694 60.0373 320.281 63.6121C315.868 67.204 310.703 69 304.784 69H304.767Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M350.368 19.3286C346.913 19.3286 345.168 21.1588 345.168 24.8192V28.9585H353.447V34.2266H345.168V67.7856H339.147V34.2266H332.75V28.9585H339.147V24.9731C339.147 17.7037 342.756 14.0605 349.991 14.0605C351.343 14.0605 352.626 14.1118 353.823 14.2144V19.3286H350.368Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M392.667 28.9585L378.59 47.9273L393.42 67.8027H386.338L384.235 64.8607C381.019 60.2938 377.991 56.0861 375.135 52.2205C370.927 57.8992 367.831 62.124 365.881 64.9462L363.846 67.8027H356.85L371.68 47.9273L357.603 28.9585H364.684L367.695 33.0977C370.654 37.1173 373.168 40.6237 375.221 43.6341C377.786 40.025 380.283 36.5015 382.746 33.0977L385.688 28.9585H392.684H392.667Z"
        fill="currentColor"
      />
      <path d="M406.282 67.7856H398.688V60.2596H406.282V67.7856Z" fill="currentColor" />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M433.906 28.3598C434.71 28.3598 435.394 28.3769 435.941 28.4282V34.2266H434.436C430.468 34.2266 427.355 35.2016 425.097 37.1686C422.839 39.1185 421.71 41.9578 421.71 45.6695V67.8027H415.69V28.9585H421.403V35.8002C424.054 30.84 428.227 28.3427 433.889 28.3427L433.906 28.3598Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M457.014 68.6921C451.643 68.6921 447.299 66.8619 443.963 63.2016C440.628 59.5412 438.952 54.598 438.952 48.372C438.952 42.146 440.594 37.3396 443.878 33.628C447.162 29.9163 451.489 28.0519 456.86 28.0519C462.231 28.0519 466.798 29.9847 469.962 33.8503C473.126 37.7159 474.7 42.5565 474.7 48.372V50.4759H445.195C445.503 54.7349 446.683 57.9334 448.77 60.0373C450.856 62.1411 453.644 63.2016 457.168 63.2016C462.539 63.2016 465.977 60.7899 467.482 55.9835H473.725C472.716 59.7978 470.8 62.8766 467.961 65.2028C465.122 67.529 461.478 68.7092 457.014 68.7092V68.6921ZM445.349 45.0538H468.235C467.927 41.7013 466.798 38.9304 464.814 36.7752C462.83 34.62 460.178 33.5425 456.877 33.5425C453.764 33.5425 451.199 34.5174 449.163 36.4844C447.128 38.4343 445.862 41.3079 445.366 45.0709L445.349 45.0538Z"
        fill="currentColor"
      />
    </svg>
  );
}

// TODO: Remove once we have that in the UI components library
function CfxLogo() {
  return (
    <svg viewBox="0 0 310 74" fill="none" xmlns="http://www.w3.org/2000/svg">
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M64.4992 5H52.0201L53.054 17.8535H41.6446L42.6785 5H30.3064L0 69H37.4908L38.7209 53.7933H55.9599L57.19 69H94.7877L64.4813 5H64.4992ZM39.5944 43.2039L41.2167 23.2017H53.4997L55.122 43.2039H39.5944Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M139.767 65C132.498 65 126.546 62.4685 121.927 57.4056C117.309 52.3256 115 45.5351 115 37C115 28.4649 117.309 21.6744 121.927 16.5944C126.546 11.5315 132.481 9 139.767 9C145.395 9 150.269 10.5394 154.409 13.584C158.548 16.6286 161.268 21.2639 162.567 27.4386H156.17C155.007 23.0257 152.989 19.81 150.081 17.8088C147.173 15.8076 143.735 14.7984 139.767 14.7984C134.294 14.7984 129.915 16.7654 126.597 20.6995C123.279 24.6335 121.637 30.0727 121.637 37C121.637 43.9273 123.296 49.3494 126.597 53.3005C129.915 57.2346 134.294 59.2016 139.767 59.2016C143.889 59.2016 147.55 57.9359 150.748 55.4044C153.964 52.8729 155.897 49.1784 156.547 44.3036H162.875C162.225 50.923 159.694 56.0373 155.281 59.6121C150.868 63.204 145.703 65 139.784 65H139.767Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M185.368 15.3286C181.913 15.3286 180.168 17.1588 180.168 20.8192V24.9585H188.447V30.2266H180.168V63.7856H174.147V30.2266H167.75V24.9585H174.147V20.9731C174.147 13.7037 177.756 10.0605 184.991 10.0605C186.343 10.0605 187.626 10.1118 188.823 10.2144V15.3286H185.368Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M227.667 24.9585L213.59 43.9273L228.42 63.8027H221.338L219.235 60.8607C216.019 56.2938 212.991 52.0861 210.135 48.2205C205.927 53.8992 202.831 58.124 200.881 60.9462L198.846 63.8027H191.85L206.68 43.9273L192.603 24.9585H199.684L202.695 29.0977C205.654 33.1173 208.168 36.6237 210.221 39.6341C212.786 36.025 215.283 32.5015 217.746 29.0977L220.688 24.9585H227.684H227.667Z"
        fill="currentColor"
      />
      <path d="M241.282 63.7856H233.688V56.2596H241.282V63.7856Z" fill="currentColor" />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M268.906 24.3598C269.71 24.3598 270.394 24.3769 270.941 24.4282V30.2266H269.436C265.468 30.2266 262.355 31.2016 260.097 33.1686C257.839 35.1185 256.71 37.9578 256.71 41.6695V63.8027H250.69V24.9585H256.403V31.8002C259.054 26.84 263.227 24.3427 268.889 24.3427L268.906 24.3598Z"
        fill="currentColor"
      />
      <path
        // eslint-disable-next-line @stylistic/max-len
        d="M292.014 64.6921C286.643 64.6921 282.299 62.8619 278.963 59.2016C275.628 55.5412 273.952 50.598 273.952 44.372C273.952 38.146 275.594 33.3396 278.878 29.628C282.162 25.9163 286.489 24.0519 291.86 24.0519C297.231 24.0519 301.798 25.9847 304.962 29.8503C308.126 33.7159 309.7 38.5565 309.7 44.372V46.4759H280.195C280.503 50.7349 281.683 53.9334 283.77 56.0373C285.856 58.1411 288.644 59.2016 292.168 59.2016C297.539 59.2016 300.977 56.7899 302.482 51.9835H308.725C307.716 55.7978 305.8 58.8766 302.961 61.2028C300.122 63.529 296.478 64.7092 292.014 64.7092V64.6921ZM280.349 41.0538H303.235C302.927 37.7013 301.798 34.9304 299.814 32.7752C297.83 30.62 295.178 29.5425 291.877 29.5425C288.764 29.5425 286.199 30.5174 284.163 32.4844C282.128 34.4343 280.862 37.3079 280.366 41.0709L280.349 41.0538Z"
        fill="currentColor"
      />
    </svg>
  );
}
