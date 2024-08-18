import {
  Avatar,
  Button,
  BrandIcon,
  Indicator,
  InfoPanel,
  Input,
  Flex,
  Separator,
  Text,
} from '@cfx-dev/ui-components';
import { observer } from 'mobx-react-lite';

import { useAccountService } from 'cfx/common/services/account/account.service';
// eslint-disable-next-line camelcase
import { $L, $L_nl2br } from 'cfx/common/services/intl/l10n';

import { IAuthFormState, totpFieldRef, useAuthFormState, usernameRegexp } from './AuthFormState';

import s from './AuthForm.module.scss';

export type AuthFormProps = Partial<Pick<IAuthFormState, 'onDone' | 'onModeChange' | 'onDisabledChange'>>;

export const AuthForm = observer(function AuthForm(props: AuthFormProps) {
  const AccountService = useAccountService();

  const state = useAuthFormState();

  // ---
  state.onDone = props.onDone;
  state.onModeChange = props.onModeChange;
  state.onDisabledChange = props.onDisabledChange;
  // ---

  if (state.isAuthenticated) {
    return (
      <Flex vertical fullWidth fullHeight centered gap="xlarge">
        <Flex centered vertical gap="xlarge">
          <Flex centered vertical gap="large">
            <Avatar size="large" url={AccountService.account?.getAvatarUrl() || ''} className={s.avatar} />

            <Flex gap="small">
              <Text size="xlarge" opacity="50">
                {$L('#AuthForm_Success_Welcome')}
              </Text>

              <Text size="xlarge">{AccountService.account?.username}</Text>
            </Flex>
          </Flex>

          <div />

          <Button
            theme="primary"
            size="large"
            text={$L('#AuthForm_Success_Continue')}
            onClick={state.continueAuthenticated}
          />
        </Flex>
      </Flex>
    );
  }

  return (
    <Flex vertical fullWidth fullHeight centered gap="xlarge">
      {state.isExternal && (
        <>
          <Indicator />

          <Text>{$L('#AuthForm_External_Instruction')}</Text>
        </>
      )}

      {state.isRegistrationActivation && (
        <>
          <Text centered typographic>
            {$L_nl2br('&AuthForm_Registration_Activation_1')}
          </Text>
          <Text centered typographic>
            {$L_nl2br('&AuthForm_Registration_Activation_2')}
          </Text>
          <Text size="small" opacity="50">
            {$L('#AuthForm_Registration_Activation_3')}
          </Text>
        </>
      )}

      {state.showExternalAuthButton && (
        <>
          <Button
            icon={BrandIcon.cfxre}
            text={$L('#AuthForm_External_Initialize')}
            className={s.authbtn}
            onClick={state.beginExternalAuth}
          />

          <Separator content={$L('#AuthForm_External_Or_Internal')} />
        </>
      )}

      <Flex vertical fullWidth gap="large">
        {state.showEmailField && (
          <Input
            autofocus
            fullWidth
            label={$L('#AuthForm_FieldLabel_Email')}
            placeholder="carl.johnson@email.com"
            value={state.email.value}
            onChange={state.email.set}
            disabled={state.disabled}
            onSubmit={state.handleSubmit}
            decorator={state.renderEmailDecorator()}
          />
        )}

        {state.showPasswordField && (
          <Input
            fullWidth
            type="password"
            label={$L('#AuthForm_FieldLabel_Password')}
            placeholder="********"
            value={state.password}
            onChange={state.setPassword}
            disabled={state.disabled}
            onSubmit={state.handleSubmit}
            decorator={state.renderPasswordDecorator()}
          />
        )}

        {state.showUsernameField && (
          <Input
            fullWidth
            label={$L('#AuthForm_FieldLabel_Username')}
            placeholder="CarlJohnson07"
            value={state.username.value}
            onChange={state.username.set}
            disabled={state.disabled}
            onSubmit={state.handleSubmit}
            pattern={usernameRegexp}
            decorator={state.renderUsernameDecorator()}
          />
        )}

        {state.showTOTPField && (
          <Input
            inputRef={totpFieldRef}
            autofocus
            fullWidth
            label={$L('#AuthForm_FieldLabel_TOTP')}
            placeholder="112233"
            value={state.totp}
            onChange={state.setTOTP}
            disabled={state.disabled}
            onSubmit={state.handleSubmit}
            description={$L('#AuthForm_FieldDescription_TOTP')}
          />
        )}

        <SubmitControls state={state} />

        {!!state.submitMessage.hasMessage && (
          <InfoPanel
            type={state.submitMessage.isError
              ? 'error'
              : 'success'}
            className={s.message}
          >
            {$L_nl2br(state.submitMessage.message)}
          </InfoPanel>
        )}
      </Flex>
    </Flex>
  );
});

interface SubmitControlsProps {
  state: IAuthFormState;
}

const SubmitControls = observer(function SubmitControls({
  state,
}: SubmitControlsProps) {
  if (state.isLogIn) {
    return (
      <Flex centered repell>
        <Button
          text={$L('#AuthForm_SwitchTo_Registration')}
          theme="transparent"
          disabled={state.disabled}
          onClick={state.switchToRegistration}
        />

        {state.disabled
          ? (
            <Indicator />
            )
          : null}

        <Button
          text={$L('#AuthForm_LogIn_Submit')}
          theme={state.disabled
            ? 'default'
            : 'primary'}
          disabled={state.disabled || !state.canSubmit}
          onClick={state.handleSubmit}
        />
      </Flex>
    );
  }

  if (state.isTOTP) {
    return (
      <Flex centered repell>
        <Button
          text={$L('#AuthForm_Back')}
          theme="transparent"
          disabled={state.disabled}
          onClick={state.switchToLogIn}
        />

        {state.disabled
          ? (
            <Indicator />
            )
          : null}

        <Button
          text={$L('#AuthForm_LogIn_Submit')}
          theme={state.disabled
            ? 'default'
            : 'primary'}
          disabled={state.disabled || !state.canSubmit}
          onClick={state.handleSubmit}
        />
      </Flex>
    );
  }

  if (state.isExternal) {
    return (
      <Flex centered>
        <Button text={$L('#AuthForm_Back')} onClick={state.switchToLogIn} />
      </Flex>
    );
  }

  if (state.isRegistrationActivation) {
    return (
      <Flex centered repell>
        <Button
          text={$L('#AuthForm_Registration_ResendActivation')}
          disabled={state.disabled}
          onClick={state.resendActivationEmail}
        />

        {state.disabled
          ? (
            <Indicator />
            )
          : null}

        <Button
          theme="primary"
          text={$L('#AuthForm_LogIn_Submit')}
          disabled={state.disabled}
          onClick={state.handleSubmit}
        />
      </Flex>
    );
  }

  return (
    <Flex centered repell>
      <Button
        text={$L('#AuthForm_SwitchTo_LogIn')}
        theme="transparent"
        disabled={state.disabled}
        onClick={state.switchToLogIn}
      />

      {state.disabled
        ? (
          <Indicator />
          )
        : null}

      <Button
        text={$L('#AuthForm_Registration_Submit')}
        theme={state.disabled
          ? 'default'
          : 'primary'}
        disabled={state.disabled || !state.canSubmit}
        onClick={state.handleSubmit}
      />
    </Flex>
  );
});
