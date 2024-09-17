import { inject, injectable, optional } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { defineService, ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { AuthFormMode, DEAFULT_AUTH_FORM_MODE } from 'cfx/common/parts/AuthForm/AuthFormState';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { IAnalyticsService } from 'cfx/common/services/analytics/analytics.service';
import { ElementPlacements, EventActionNames } from 'cfx/common/services/analytics/types';
import { createEnumChecker } from 'cfx/utils/enum';

export const IAuthService = defineService<IAuthService>('AuthService');
export type IAuthService = AuthService;

export function registerAuthService(container: ServicesContainer) {
  container.registerImpl(IAuthService, AuthService);
}

export function useAuthService(): IAuthService {
  return useService(IAuthService);
}

enum AuthModalMementoState {
  INITIAL = 'initial',
  SHOWN = 'shown',
  IGNORE = 'ignore',
}

const authModalMementoStateChecker = createEnumChecker(AuthModalMementoState);

namespace LS_KEYS {
  export const AUTH_MODAL_MEMEMORIZED_STATE = 'discourseAuthModalState';
}

@injectable()
export class AuthService {
  @inject(IAnalyticsService)
  @optional()
  protected readonly analyticsService: IAnalyticsService | void;

  private _UIOpen: boolean = false;
  public get UIOpen(): boolean {
    return this._UIOpen;
  }
  private set UIOpen(UIOpen: boolean) {
    this._UIOpen = UIOpen;
  }

  private _mementoState = getSavedMementoState();
  private get mementoState(): AuthModalMementoState {
    return this._mementoState;
  }
  private set mementoState(state: AuthModalMementoState) {
    this._mementoState = state;
  }

  private _authFormMode = DEAFULT_AUTH_FORM_MODE;

  private _authFormDisabled: boolean = false;
  public get authFormDisabled(): boolean {
    return this._authFormDisabled;
  }
  private set authFormDisabled(authFormDisabled: boolean) {
    this._authFormDisabled = authFormDisabled;
  }

  get isAuthenticated(): boolean {
    return this._authFormMode === AuthFormMode.Authenticated;
  }

  get showDismissAndIgnoreNextTime(): boolean {
    if (this._authFormMode === AuthFormMode.External || this.isAuthenticated) {
      return false;
    }

    return this.mementoState !== AuthModalMementoState.IGNORE;
  }

  constructor(
    @inject(IAccountService)
    protected readonly accountService: IAccountService,
  ) {
    makeAutoObservable(this);

    accountService.initialAuthCompletePromise().then((authenticated) => {
      if (authenticated) {
        return;
      }

      if (this._mementoState === AuthModalMementoState.IGNORE) {
        return;
      }

      this.UIOpen = true;
    });
  }

  readonly openUI = () => {
    this.UIOpen = true;
  };

  readonly closeUI = () => {
    this.UIOpen = false;
  };

  readonly handleAuthFormDisabled = (disabled: boolean) => {
    this.authFormDisabled = disabled;
  };

  readonly dismiss = () => {
    this.UIOpen = false;

    this.saveMementoState(AuthModalMementoState.SHOWN);

    if (!this.isAuthenticated) {
      this.trackAuthModalDismissed(false);
    }
  };

  readonly dismissIgnoreNextTime = () => {
    this.UIOpen = false;

    this.saveMementoState(AuthModalMementoState.IGNORE);
    this.trackAuthModalDismissed(true);
  };

  readonly handleAuthFormModeChange = (mode: AuthFormMode) => {
    this._authFormMode = mode;
  };

  readonly handleAuthFormDone = () => {
    this.UIOpen = false;
  };

  private saveMementoState(state: AuthModalMementoState) {
    this.mementoState = state;
    window.localStorage.setItem(LS_KEYS.AUTH_MODAL_MEMEMORIZED_STATE, state);
  }

  private trackAuthModalDismissed(ignored: boolean) {
    if (!this.analyticsService) {
      return;
    }

    this.analyticsService.trackEvent({
      action: EventActionNames.CTAOther,
      properties: {
        element_placement: ElementPlacements.AuthModal,
        text: `Closed ${
          ignored
            ? 'And Ignored'
            : ''
        } ${authFormModeToAnalyticsLabel[this._authFormMode]}`,
        link_url: '',
      },
    });
  }
}

const authFormModeToAnalyticsLabel: Record<AuthFormMode, string> = {
  [AuthFormMode.LogIn]: 'LoginScreen',
  [AuthFormMode.TOTP]: 'LoginScreen',
  [AuthFormMode.Registration]: 'RegistrationScreen',
  [AuthFormMode.RegistrationActivation]: 'RegistrationActivationScreen',
  [AuthFormMode.External]: 'ExternalAuthScreen',
  [AuthFormMode.Authenticated]: 'AuthenticatedScreen',
};

function getSavedMementoState(): AuthModalMementoState {
  const savedValue = window.localStorage.getItem(LS_KEYS.AUTH_MODAL_MEMEMORIZED_STATE) || AuthModalMementoState.INITIAL;

  if (authModalMementoStateChecker(savedValue)) {
    return savedValue;
  }

  return AuthModalMementoState.INITIAL;
}
