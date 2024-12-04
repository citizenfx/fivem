/* eslint-disable camelcase */
import { inject, injectable } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { useServiceResolver } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { ExternalAuthCompleteEvent } from 'cfx/common/services/account/events';
import { dispose, Disposer } from 'cfx/utils/disposable';
import { useDisposableInstance } from 'cfx/utils/hooks';

import { IAuthService } from '../../services/auth/auth.service';
import { AuthFormMode } from '../../services/auth/constants';

export type IAuthFormState = AuthFormState;
export function useAuthFormState(): AuthFormState {
  return useDisposableInstance(initAuthFormState, useServiceResolver());
}
function initAuthFormState(serviceResolver: ReturnType<typeof useServiceResolver>): AuthFormState {
  return serviceResolver(AuthFormState);
}

@injectable()
class AuthFormState {
  private _mode = AuthFormMode.Initial;
  public get mode(): AuthFormMode {
    return this._mode;
  }
  private set mode(mode: AuthFormMode) {
    this._mode = mode;
  }

  private _errorMessage = '';
  public get errorMessage(): string {
    return this._errorMessage;
  }
  private set errorMessage(errorMessage: string) {
    this._errorMessage = errorMessage;
  }

  public get disabled(): boolean {
    if (this.isModeExternalAuthInitialized) {
      return true;
    }

    if (this.isModeExternalAuthProcessing) {
      return true;
    }

    return false;
  }

  get isModeInitial(): boolean {
    return this.mode === AuthFormMode.Initial;
  }

  get isModeExternalAuthInitialized(): boolean {
    return this.mode === AuthFormMode.ExternalAuthInitialized;
  }

  get isModeExternalAuthProcessing(): boolean {
    return this.mode === AuthFormMode.ExternalAuthProcessing;
  }

  get isModeAuthenticated(): boolean {
    return this.mode === AuthFormMode.Authenticated;
  }

  private toDispose: Disposer;

  constructor(
    @inject(IAccountService)
    protected readonly accountService: IAccountService,
    @inject(IAuthService)
    protected readonly authService: IAuthService,
  ) {
    makeAutoObservable(this);

    this.toDispose = new Disposer();

    this.toDispose.add(
      this.accountService.ExternalAuthProcessing.addListener(this.switchToModeExternalAuthProcessing),
      this.accountService.ExternalAuthComplete.addListener(this.handleExternalAuthComplete),
    );
  }

  public readonly dispose = () => {
    dispose(this.toDispose);
  };

  public readonly beginExternalAuth = async () => {
    this.mode = AuthFormMode.ExternalAuthInitialized;

    this.accountService.initializeExternalAuth();
  };

  public readonly continueAuthenticated = async () => {
    if (this.mode !== AuthFormMode.Authenticated) {
      return;
    }

    if (!this.accountService.account) {
      return;
    }

    this.authService.handleAuthFormDone();
  };

  public readonly switchToModeInitial = () => {
    this.mode = AuthFormMode.Initial;
    this.errorMessage = '';
  };

  private readonly switchToModeExternalAuthProcessing = () => {
    this.mode = AuthFormMode.ExternalAuthProcessing;
    this.errorMessage = '';
  };

  private readonly switchToModeAuthenticated = () => {
    this.mode = AuthFormMode.Authenticated;
    this.errorMessage = '';
  };

  private readonly handleExternalAuthComplete = (event: ExternalAuthCompleteEvent) => {
    if (event.success) {
      this.switchToModeAuthenticated();

      return;
    }

    this.errorMessage = event.error || '#AuthForm_External_Failed';
  };
}
