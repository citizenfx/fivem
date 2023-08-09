import { ServicesContainer, defineService, useService } from "cfx/base/servicesContainer";
import { ASID } from "cfx/utils/asid";
import { joaat } from "cfx/utils/hash";
import { injectable } from "inversify";
import { makeAutoObservable } from "mobx";

export const ILegalService = defineService<LegalService>('legalService');

export function registerLegalService(container: ServicesContainer) {
  container.registerImpl(ILegalService, LegalService);
}

export function useLegalService() {
  return useService(ILegalService);
}

const LS_KEY = 'legalAcceptanceData';

type TOSVersionHash = number;
type AcceptanceTimestamp = number;

type LegalAcceptanceData = [AcceptanceTimestamp, TOSVersionHash];

@injectable()
export class LegalService {
  readonly CURRENT_TOS_VERSION = '2020-09-06';
  readonly TOS_URL = 'https://runtime.fivem.net/fivem-service-agreement-4.pdf';

  private readonly currentTOSVersionHash = joaat(`${ASID}.${this.CURRENT_TOS_VERSION}`);

  private _hasUserAccepted: boolean;
  public get hasUserAccepted(): boolean { return this._hasUserAccepted }
  private set hasUserAccepted(hasUserAccepted: boolean) { this._hasUserAccepted = hasUserAccepted }

  constructor() {
    this._hasUserAccepted = this.reviveHasUserAccepted();

    makeAutoObservable(this);
  }

  public readonly accept = () => {
    try {
      const data: LegalAcceptanceData = [
        Date.now(),
        this.currentTOSVersionHash,
      ];

      window.localStorage.setItem(LS_KEY, JSON.stringify(data));

      this.hasUserAccepted = true;
    } catch (e) {
      // no-op
    }
  };

  private reviveHasUserAccepted(): boolean {
    try {
      const [_timestamp, acceptedTOSVersionHash] = JSON.parse(window.localStorage.getItem(LS_KEY) || '[]') as LegalAcceptanceData;

      return acceptedTOSVersionHash === this.currentTOSVersionHash;
    } catch (e) {
      return false;
    }
  }
}
