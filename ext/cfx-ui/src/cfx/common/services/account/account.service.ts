import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';

import { AccountChangeEvent, ExternalAuthCompleteEvent } from './events';
import { IAccount } from './types';
import { defineService, useService } from '../../../base/servicesContainer';

export const IAccountService = defineService<IAccountService>('AccountService');
export interface IAccountService {
  readonly account: IAccount | null;
  readonly accountLoadError: string | null;
  readonly accountLoadComplete: boolean;

  initialAuthCompletePromise(): Promise<boolean>;

  readonly accountChange: SingleEventEmitter<AccountChangeEvent>;

  initializeExternalAuth(): void;
  readonly ExternalAuthProcessing: SingleEventEmitter<void>;
  readonly ExternalAuthComplete: SingleEventEmitter<ExternalAuthCompleteEvent>;

  signout(): void;

  getAvatarUrlForUser(template: string, size?: number): string;

  getSignUpURL(): string;
}

export function useAccountService(): IAccountService {
  return useService(IAccountService);
}
