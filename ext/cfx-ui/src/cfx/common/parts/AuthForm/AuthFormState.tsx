import React from "react";
import { Icons } from "cfx/ui/Icons";
import { Indicator } from "cfx/ui/Indicator/Indicator";
import { Text } from "cfx/ui/Text/Text";
import { makeAutoObservable, reaction } from "mobx";
import { Button } from "cfx/ui/Button/Button";
import { LoginStatus, RegisterStatus } from "cfx/common/services/account/types";
import { Optional } from "cfx/utils/types";
import { dispose, Disposer, IDisposableObject } from "cfx/utils/disposable";
import { SSOAuthCompleteEvent } from "cfx/common/services/account/events";
import { OnlyLatest } from "cfx/utils/async";
import { returnTrue } from "cfx/utils/functional";
import { inject, injectable } from "inversify";
import { IAccountService } from "cfx/common/services/account/account.service";
import { useDisposableInstance } from "cfx/utils/hooks";
import { useServiceResolver } from "cfx/base/servicesContainer";
import { currentGameNameIs } from "cfx/base/gameRuntime";
import { GameName } from "cfx/base/game";
import { LocaleKeyOrString, LocaleKeyOrString_nl2br } from "cfx/common/services/intl/types";
import { $L } from "cfx/common/services/intl/l10n";

export type IAuthFormState = AuthFormState;
export function useAuthFormState(): AuthFormState {
  return useDisposableInstance(initAuthFormState, useServiceResolver());
}
function initAuthFormState(serviceResolver: ReturnType<typeof useServiceResolver>): AuthFormState {
  return serviceResolver(AuthFormState);
}

export const usernameRegexp = /^[A-Za-z0-9._-]*$/;

export const totpFieldRef = React.createRef<HTMLInputElement>();

export enum AuthFormMode {
  LogIn,
  Registration,
  RegistrationActivation,
  TOTP,
  External,
  Authenticated,
}

export const DEAFULT_AUTH_FORM_MODE = AuthFormMode.LogIn;

@injectable()
class AuthFormState {
  public onDone: Optional<() => void>;
  public onModeChange: Optional<(mode: AuthFormMode) => void>;
  public onDisabledChange: Optional<(disabled: boolean) => void>;

  private _mode = DEAFULT_AUTH_FORM_MODE;
  public get mode(): AuthFormMode { return this._mode }
  private set mode(mode: AuthFormMode) { this._mode = mode }

  public readonly username: FieldController;
  public readonly email: FieldController;

  private _password = '';
  public get password(): string { return this._password }
  private set password(password: string) { this._password = password }

  private _passwordResetPending = false;
  public get passwordResetPending(): boolean { return this._passwordResetPending }
  private set passwordResetPending(pending: boolean) { this._passwordResetPending = pending }

  // Two-Factor authentication code
  private _totp = '';
  public get totp(): string { return this._totp }
  private set totp(totp: string) { this._totp = totp }

  private _submitPending = false;
  public get submitPending(): boolean { return this._submitPending }
  public set submitPending(pending: boolean) { this._submitPending = pending }

  public readonly submitMessage: SubmitMessage;

  public get disabled(): boolean {
    if (this.isExternal) {
      return true;
    }

    if (this.passwordResetPending) {
      return true;
    }

    if (this.submitPending) {
      return true;
    }

    return false;
  }

  get showExternalAuthButton(): boolean {
    if (!currentGameNameIs(GameName.FiveM)) {
      return false;
    }

    return this.isLogIn;
  }

  get showEmailField(): boolean {
    return this.isLogIn || this.isRegistration;
  }
  get showPasswordField(): boolean {
    return this.isLogIn || this.isRegistration;
  }
  get showUsernameField(): boolean {
    return this.isRegistration;
  }
  get showTOTPField(): boolean {
    return this.isTOTP;
  }

  get hasValidEmail(): boolean {
    return this.email.isValid;
  }
  get hasValidUsername(): boolean {
    return this.username.isValid;
  }

  get canSubmit(): boolean {
    if (this.email.validationPending || this.username.validationPending) {
      return false;
    }

    if (this.mode === AuthFormMode.LogIn) {
      return this.hasValidEmail && !!this.password;
    }

    if (this.mode === AuthFormMode.Registration) {
      return this.hasValidEmail && this.hasValidUsername && !!this.password;
    }

    return !!this.totp;
  }

  get canRequestPasswordReset(): boolean {
    if (this.mode === AuthFormMode.Registration) {
      return false;
    }

    return this.hasValidEmail;
  }

  get isLogIn(): boolean {
    return this.mode === AuthFormMode.LogIn;
  }
  get isRegistration(): boolean {
    return this.mode === AuthFormMode.Registration;
  }
  get isRegistrationActivation(): boolean {
    return this.mode === AuthFormMode.RegistrationActivation;
  }
  get isTOTP(): boolean {
    return this.mode === AuthFormMode.TOTP;
  }
  get isExternal(): boolean {
    return this.mode === AuthFormMode.External;
  }
  get isAuthenticated(): boolean {
    return this.mode === AuthFormMode.Authenticated;
  }

  private toDispose: Disposer;

  constructor(
    @inject(IAccountService)
    protected readonly accountService: IAccountService,
  ) {
    makeAutoObservable(this);

    this.submitMessage = new SubmitMessage();
    this.toDispose = new Disposer();

    this.username = this.toDispose.register(new FieldController(
      (username) => this.accountService.getUsernameError(username),
      (newUsername) => usernameRegexp.test(newUsername),
    ));

    this.email = this.toDispose.register(new FieldController(
      (email) => this.accountService.getEmailError(email),
    ));

    this.toDispose.add(
      reaction(() => this.disabled, (disabled) => this.onDisabledChange?.(disabled)),
      reaction(() => this.mode, (mode) => this.onModeChange?.(mode)),

      this.accountService.SSOAuthComplete.addListener(this.handleExternalAuthComplete),
    );
  }

  public readonly dispose = () => {
    dispose(this.toDispose);
  };

  public readonly setPassword = (password: string) => {
    this.password = password;
  };

  public readonly setTOTP = (totp: string) => {
    this.totp = totp;
  };

  public readonly handleSubmit = async () => {
    this.submitPending = true;
    this.submitMessage.reset();

    try {
      switch (this.mode) {
        case AuthFormMode.TOTP:
        case AuthFormMode.LogIn:
        case AuthFormMode.RegistrationActivation: {
          const response = await this.accountService.login({
            email: this.email.value,
            password: this.password,
            totp: this.totp,
          });

          switch (response.status) {
            case LoginStatus.Success: {
              return this.switchToAuthenticated();
            }

            case LoginStatus.Error: {
              this.submitMessage.setError(response.error);
              return;
            }

            case LoginStatus.TOTPRequest: {
              this._mode = AuthFormMode.TOTP;

              // hacky as autofocus doesn't work in this case, sadly
              requestAnimationFrame(() => {
                totpFieldRef.current?.focus();
              });

              return;
            }
          }

          break;
        }

        case AuthFormMode.Registration: {
          const response = await this.accountService.register({
            email: this.email.value,
            password: this.password,
            username: this.username.value,
          });

          if (response.status === RegisterStatus.Success) {
            this.switchToRegistrationActivation();
          } else {
            this.submitMessage.setError(response.error);
          }

          break;
        }
      }
    } finally {
      this.submitPending = false;
    }
  };

  public readonly beginExternalAuth = async () => {
    this.mode = AuthFormMode.External;

    this.accountService.initializeSSOAuth();
  };

  public readonly continueAuthenticated = async () => {
    if (this.mode !== AuthFormMode.Authenticated) {
      return;
    }

    if (!this.accountService.account) {
      return;
    }

    this.onDone?.();
  };

  public readonly resendActivationEmail = async () => {
    this.submitPending = true;

    try {
      await this.accountService.resendActivationEmail(this.username.value);

      this.submitMessage.setSuccess('#AuthForm_Registration_ActivationSent');
    } catch (e) {
      this.submitMessage.setError('#AuthForm_Registration_ActivationError');
    }

    this.submitPending = false;
  };

  public readonly switchToLogIn = () => {
    this.mode = AuthFormMode.LogIn;
    this.totp = '';
    this.submitMessage.reset();
  };
  public readonly switchToRegistration = () => {
    this.mode = AuthFormMode.Registration;
    this.totp = '';
    this.submitMessage.reset();
  };
  private readonly switchToRegistrationActivation = () => {
    this.mode = AuthFormMode.RegistrationActivation;
    this.submitMessage.reset();
  };
  private readonly switchToAuthenticated = () => {
    this.mode = AuthFormMode.Authenticated;
    this.submitMessage.reset();
  };

  public readonly renderEmailDecorator = () => {
    if (this.email.validationPending) {
      return (
        <Indicator />
      );
    }

    if (this.email.hasError) {
      return (
        <Text weight="bold" color="error">
          {this.email.error}
        </Text>
      );
    }

    if (this.email.value) {
      return (
        <Text weight="bold" color="success">
          {Icons.checkmark}
        </Text>
      );
    }

    return null;
  };

  public readonly renderUsernameDecorator = () => {
    if (this.username.validationPending) {
      return (
        <Indicator />
      );
    }

    if (this.username.hasError) {
      return (
        <Text weight="bold" color="error">
          {this.username.error}
        </Text>
      );
    }

    if (this.username.value) {
      return (
        <Text weight="bold" color="success">
          {Icons.checkmark}
        </Text>
      );
    }

    return null;
  };

  public readonly renderPasswordDecorator = () => {
    if (this.canRequestPasswordReset) {
      return (
        <Button
          text={$L('#AuthForm_PasswordReset_Submit')}
          size="small"
          theme="transparent"
          disabled={this.disabled || this.passwordResetPending}
          onClick={this.requestPasswordReset}
        />
      );
    }

    return null;
  };

  public readonly requestPasswordReset = async () => {
    this.passwordResetPending = true;

    try {
      await this.accountService.resetPassword(this.email.value);

      this.submitMessage.setSuccess('#AuthForm_PasswordReset_Sent');
    } catch (e) {
      this.submitMessage.setError('#AuthForm_PasswordReset_Error');
    }

    this.passwordResetPending = false;
  };

  private readonly handleExternalAuthComplete = (event: SSOAuthCompleteEvent) => {
    if (event.success) {
      return this.switchToAuthenticated();
    }

    this.switchToLogIn();
    this.submitMessage.setError(event.error || '#AuthForm_External_Failed');
  };
}

class FieldController implements IDisposableObject {
  private _value: string = '';
  public get value(): string { return this._value }
  private set value(value: string) { this._value = value }

  private _error: string | null = null;
  public get error(): string | null { return this._error }
  private set error(error: string | null) { this._error = error }

  private _validationPending: boolean = false;
  public get validationPending(): boolean { return this._validationPending }
  private set validationPending(validationPending: boolean) { this._validationPending = validationPending }

  private readonly validatorRunner: OnlyLatest<[string], string | null>;
  private toDispose: Disposer;

  public get isValid(): boolean {
    return !!this.value && !this.error;
  }

  public get hasError(): boolean {
    return !!this.error;
  }

  constructor(
    private validator: (value: string) => Promise<string | null>,
    private shouldSet: (newValue: string, oldValue: string) => boolean = returnTrue,
    validatorDelay = 1000,
  ) {
    makeAutoObservable(this);

    this.toDispose = new Disposer();

    this.validatorRunner = this.toDispose.register(new OnlyLatest(
      this.validator,
      (error) => {
        this.error = error;
        this.validationPending = false;
      },
      validatorDelay,
    ));
  }

  public dispose() {
    dispose(this.toDispose);
  }

  public readonly set = async (newValue: string) => {
    if (this.value === newValue) {
      return;
    }

    if (!this.shouldSet(newValue, this.value)) {
      return;
    }

    this.validationPending = true;

    this.value = newValue;

    this.validatorRunner.run(newValue);
  };
}

class SubmitMessage {
  private _error = true;
  private _message = '';

  public get isError(): boolean {
    return this._error;
  }

  public get message(): string {
    return this._message;
  }

  public get hasMessage(): boolean {
    return Boolean(this._message);
  }

  public get hasError(): boolean {
    return this._error && this.hasMessage;
  }

  constructor() {
    makeAutoObservable(this);
  }

  reset() {
    this._error = true;
    this._message = '';
  }

  setError<T>(message: LocaleKeyOrString_nl2br<T> | LocaleKeyOrString<T>) {
    this._error = true;
    this._message = message;
  }

  setSuccess<T>(message: LocaleKeyOrString_nl2br<T> | LocaleKeyOrString<T>) {
    this._error = false;
    this._message = message;
  }
}
