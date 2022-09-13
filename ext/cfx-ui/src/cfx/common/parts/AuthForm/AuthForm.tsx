import { observer } from 'mobx-react-lite';
import { Button } from 'cfx/ui/Button/Button';
import { BrandIcon } from 'cfx/ui/Icons';
import { Indicator } from 'cfx/ui/Indicator/Indicator';
import { Input } from 'cfx/ui/Input/Input';
import { Separator } from 'cfx/ui/Separator/Separator';
import { Text } from 'cfx/ui/Text/Text';
import { Flex } from 'cfx/ui/Layout/Flex/Flex';
import { IAuthFormState, totpFieldRef, useAuthFormState, usernameRegexp } from './AuthFormState';
import { InfoPanel } from 'cfx/ui/InfoPanel/InfoPanel';
import { nl2brx } from 'cfx/utils/nl2br';
import { Avatar } from 'cfx/ui/Avatar/Avatar';
import { useAccountService } from 'cfx/common/services/account/account.service';
import s from './AuthForm.module.scss';

export type AuthFormProps = Partial<Pick<
  IAuthFormState,
  | 'onDone'
  | 'onModeChange'
  | 'onDisabledChange'
>>

export const AuthForm = observer(function AuthForm(props: AuthFormProps) {
  const AccountService = useAccountService();

  const state = useAuthFormState();

  {
    state.onDone = props.onDone;
    state.onModeChange = props.onModeChange;
    state.onDisabledChange = props.onDisabledChange;
  }

  if (state.isAuthenticated) {
    return (
      <Flex vertical fullWidth fullHeight centered gap="xlarge">
        <Flex centered vertical gap="xlarge">
          <Flex centered vertical gap="large">
            <Avatar
              size="large"
              url={AccountService.account?.getAvatarUrl() || ''}
              className={s.avatar}
            />

            <Flex gap="small">
              <Text size="xlarge" opacity="50">
                Welcome,
              </Text>

              <Text size="xlarge">
                {AccountService.account?.username}
              </Text>
            </Flex>
          </Flex>

          <div />

          <Button
            theme="primary"
            size="large"
            text="Continue"
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

          <Text>
            Click Authorize in Browser
          </Text>
        </>
      )}

      {state.isRegistrationActivation && (
        <>
          <Text centered typographic>
            We've sent you an activation email,
            <br />
            which should arrive shortly.
          </Text>
          <Text centered typographic>
            Once you're done activating your account,
            <br />
            click the log in button.
          </Text>
          <Text size="small" opacity="50">
            Make sure to check your spam folder if you don't see it.
          </Text>
        </>
      )}

      {state.showExternalAuthButton && (
        <>
          <Button
            icon={BrandIcon.cfxre}
            text="Log in via browser"
            className={s.authbtn}
            onClick={state.beginExternalAuth}
          />

          <Separator content="or enter credentials" />
        </>
      )}

      <Flex vertical fullWidth gap="large">
        {state.showEmailField && (
          <Input
            autofocus
            fullWidth
            label="Email"
            placeholder="davy.jones@email.com"
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
            label="Password"
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
            label="Username"
            placeholder="Playerium"
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
            label="Two-Factor Authentication"
            placeholder="112233"
            value={state.totp}
            onChange={state.setTOTP}
            disabled={state.disabled}
            onSubmit={state.handleSubmit}
            description="Enter code from your authentication app"
          />
        )}

        <SubmitControls state={state} />

        {!!state.submitMessage.hasMessage && (
          <InfoPanel
            type={state.submitMessage.isError ? 'error' : 'success'}
            className={s.message}
          >
            {nl2brx(state.submitMessage.message)}
          </InfoPanel>
        )}
      </Flex>
    </Flex>
  );
});

interface SubmitControlsProps {
  state: IAuthFormState,
}

const SubmitControls = observer(function SubmitControls({ state }: SubmitControlsProps) {
  if (state.isLogIn) {
    return (
      <Flex centered repell>
        <Button
          text="Don't have account? Register"
          theme="transparent"
          disabled={state.disabled}
          onClick={state.switchToRegistration}
        />

        {state.disabled ? <Indicator /> : null}

        <Button
          text="Log in"
          theme={state.disabled ? 'default' : 'primary'}
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
          text="Back"
          theme="transparent"
          disabled={state.disabled}
          onClick={state.switchToLogIn}
        />

        {state.disabled ? <Indicator /> : null}

        <Button
          text="Log in"
          theme={state.disabled ? 'default' : 'primary'}
          disabled={state.disabled || !state.canSubmit}
          onClick={state.handleSubmit}
        />
      </Flex>
    );
  }

  if (state.isExternal) {
    return (
      <Flex centered>
        <Button
          text="Back"
          onClick={state.switchToLogIn}
        />
      </Flex>
    );
  }

  if (state.isRegistrationActivation) {
    return (
      <Flex centered repell>
        <Button
          text="Resend activation email"
          disabled={state.disabled}
          onClick={state.resendActivationEmail}
        />

        {state.disabled ? <Indicator /> : null}

        <Button
          theme="primary"
          text="Log in"
          disabled={state.disabled}
          onClick={state.handleSubmit}
        />
      </Flex>
    );
  }

  return (
    <Flex centered repell>
      <Button
        text="Already have account? Log in"
        theme="transparent"
        disabled={state.disabled}
        onClick={state.switchToLogIn}
      />

      {state.disabled ? <Indicator /> : null}

      <Button
        text="Register"
        theme={state.disabled ? 'default' : 'primary'}
        disabled={state.disabled || !state.canSubmit}
        onClick={state.handleSubmit}
      />
    </Flex>
  );
});
