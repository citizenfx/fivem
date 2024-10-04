import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';

import { AccountChangeEvent, SSOAuthCompleteEvent } from './events';
import { IAccount, ILoginResponse, IRegisterResponse, ILoginCredentials, IRegisterCredentials } from './types';
import { defineService, useService } from '../../../base/servicesContainer';

export const IAccountService = defineService<IAccountService>('AccountService');
export interface IAccountService {
  readonly account: IAccount | null;
  readonly accountLoadError: string | null;
  readonly accountLoadComplete: boolean;

  initialAuthCompletePromise(): Promise<boolean>;

  readonly accountChange: SingleEventEmitter<AccountChangeEvent>;

  initializeSSOAuth(): void;
  readonly SSOAuthComplete: SingleEventEmitter<SSOAuthCompleteEvent>;

  register(credentials: IRegisterCredentials): Promise<IRegisterResponse>;
  login(credentials: ILoginCredentials): Promise<ILoginResponse>;
  signout(): void;

  resetPassword(email: string): void | Promise<void>;
  resendActivationEmail(username: string): void | Promise<void>;

  getEmailError(email: string, onlineCheck: boolean): Promise<string | null>;
  getUsernameError(username: string): Promise<string | null>;

  getAvatarUrlForUser(template: string, size?: number): string;
}

export function useAccountService(): IAccountService {
  return useService(IAccountService);
}
