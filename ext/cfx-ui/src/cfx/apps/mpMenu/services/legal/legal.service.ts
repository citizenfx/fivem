import { injectable } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { ServicesContainer, defineService, useService } from 'cfx/base/servicesContainer';
import { ASID } from 'cfx/utils/asid';
import { joaat } from 'cfx/utils/hash';

export const ILegalService = defineService<LegalService>('legalService');

export function registerLegalService(container: ServicesContainer) {
  container.registerImpl(ILegalService, LegalService);
}

export function useLegalService() {
  return useService(ILegalService);
}

const LS_KEY = 'legalAcceptanceData';

type LegalAcceptanceData = {
  tos: {
    acceptedAtTimestamp: number;
    versionHash: number;
  };
};

@injectable()
export class LegalService {
  readonly CURRENT_TOS_VERSION = 'September 12, 2023';

  readonly TOS_URL = 'https://fivem.net/terms';

  private readonly currentTOSVersionHash = joaat(`${ASID}.${this.CURRENT_TOS_VERSION}`);

  private _hasUserAccepted: boolean;
  public get hasUserAccepted(): boolean {
    return this._hasUserAccepted;
  }
  private set hasUserAccepted(hasUserAccepted: boolean) {
    this._hasUserAccepted = hasUserAccepted;
  }

  constructor() {
    this._hasUserAccepted = this.reviveHasUserAccepted();

    makeAutoObservable(this);
  }

  public readonly accept = () => {
    try {
      const data: LegalAcceptanceData = {
        tos: {
          acceptedAtTimestamp: Date.now(),
          versionHash: this.currentTOSVersionHash,
        },
      };

      window.localStorage.setItem(LS_KEY, btoa(JSON.stringify(data)));
    } catch (e) {
      // no-op
    } finally {
      // Set as accept for now anyway so we don't get stuck
      // this effectively means that we'll present user with ToS accepting UI on the next launch
      this.hasUserAccepted = true;
    }
  };

  private reviveHasUserAccepted(): boolean {
    try {
      const legalAcceptanceData: LegalAcceptanceData = JSON.parse(atob(window.localStorage.getItem(LS_KEY)!));

      if (legalAcceptanceData.tos.acceptedAtTimestamp <= 0) {
        return false;
      }

      return legalAcceptanceData.tos.versionHash === this.currentTOSVersionHash;
    } catch (e) {
      return false;
    }
  }
}
