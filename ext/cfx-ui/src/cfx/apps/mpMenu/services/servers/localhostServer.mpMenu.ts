import { inject, injectable } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { DEFAULT_SERVER_PORT } from 'cfx/base/serverUtils';
import { ServicesContainer } from 'cfx/base/servicesContainer';
import { scopedLogger, ScopedLogger } from 'cfx/common/services/log/scopedLogger';

import { getLocalhostServerInfo } from './source/fetchers';
import { IConvarService, KnownConvars } from '../convars/convars.service';

const CHECK_INTERVAL = 30000;

export function registerMpMenuLocalhostServerService(container: ServicesContainer) {
  container.register(MpMenuLocalhostServerService);
}

@injectable()
export class MpMenuLocalhostServerService {
  private _address: string = '';
  public get address(): string {
    return this._address;
  }
  private set address(address: string) {
    this._address = address;
  }

  private _displayName: string = '';
  public get displayName(): string {
    return this._displayName;
  }
  private set displayName(displayName: string) {
    this._displayName = displayName;
  }

  constructor(
    @inject(IConvarService)
    protected readonly convarService: IConvarService,
    @scopedLogger('MpMenuLocalhostServerService')
    protected readonly logger: ScopedLogger,
  ) {
    makeAutoObservable(this);

    this.startChecking();
  }

  private async startChecking() {
    await this.convarService.whenPopulated();

    this.checkIfAvailable();

    setInterval(this.checkIfAvailable, CHECK_INTERVAL);
  }

  private readonly checkIfAvailable = async () => {
    try {
      const serverInfo = await getLocalhostServerInfo(
        this.convarService.get(KnownConvars.localhostPort) || DEFAULT_SERVER_PORT,
      );
      this.address = serverInfo?.addr || '';
      this.displayName = serverInfo?.address || '';
    } catch (e) {
      this.address = '';
      this.displayName = '';
    }
  };
}
