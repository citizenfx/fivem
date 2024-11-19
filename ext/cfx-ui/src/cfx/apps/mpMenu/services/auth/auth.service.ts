import { inject, injectable, optional } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { defineService, ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { IAccountService } from 'cfx/common/services/account/account.service';
import { IAnalyticsService } from 'cfx/common/services/analytics/analytics.service';
import { ElementPlacements, EventActionNames } from 'cfx/common/services/analytics/types';
import { createEnumChecker } from 'cfx/utils/enum';

import { AuthFormMode, authFormModeToAnalyticsLabel } from './constants';

export const IAuthService = defineService<IAuthService>('AuthService');
export type IAuthService = AuthService;

export function registerAuthService(container: ServicesContainer) {
  container.registerImpl(IAuthService, AuthService);
}

export function useAuthService(): IAuthService {
  return useService(IAuthService);
}

enum AuthUINudgeState {
  INITIAL = 'initial',
  SHOWN = 'shown',
  IGNORE = 'ignore',
}

const authUINudgeStateChecker = createEnumChecker(AuthUINudgeState);

namespace LS_KEYS {
  export const AUTH_UI_NUDGE_STATE = 'discourseAuthModalState';
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

  private _uiNudgeState = getSavedAuthUINudgeState();
  private get uiNudgeState(): AuthUINudgeState {
    return this._uiNudgeState;
  }
  private set uiNudgeState(state: AuthUINudgeState) {
    this._uiNudgeState = state;
  }

  get showDismissAndIgnoreNextTime(): boolean {
    return this.uiNudgeState !== AuthUINudgeState.IGNORE;
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

      if (this._uiNudgeState === AuthUINudgeState.IGNORE) {
        return;
      }

      this.UIOpen = true;
    });
  }

  readonly openAuthUI = () => {
    this.UIOpen = true;
  };

  readonly closeAuthUI = () => {
    this.UIOpen = false;
  };

  readonly dismissAuthUI = (formMode: AuthFormMode) => {
    this.UIOpen = false;

    this.saveMementoState(AuthUINudgeState.SHOWN);

    if (!this.accountService.account) {
      this.trackAuthModalDismissed(false, formMode);
    }
  };

  readonly dismissAuthUIAndIgnoreNextTime = (formMode: AuthFormMode) => {
    this.UIOpen = false;

    this.saveMementoState(AuthUINudgeState.IGNORE);
    this.trackAuthModalDismissed(true, formMode);
  };

  readonly handleAuthFormDone = () => {
    this.UIOpen = false;
  };

  private saveMementoState(state: AuthUINudgeState) {
    this.uiNudgeState = state;
    window.localStorage.setItem(LS_KEYS.AUTH_UI_NUDGE_STATE, state);
  }

  private trackAuthModalDismissed(ignored: boolean, formMode: AuthFormMode) {
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
        } ${authFormModeToAnalyticsLabel[formMode]}`,
        link_url: '',
      },
    });
  }
}

function getSavedAuthUINudgeState(): AuthUINudgeState {
  const savedValue = window.localStorage.getItem(LS_KEYS.AUTH_UI_NUDGE_STATE) || AuthUINudgeState.INITIAL;

  if (authUINudgeStateChecker(savedValue)) {
    return savedValue;
  }

  return AuthUINudgeState.INITIAL;
}
