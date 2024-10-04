import { inject, injectable, named } from 'inversify';
import { makeAutoObservable } from 'mobx';

import { ServicesContainer } from 'cfx/base/servicesContainer';
import { IIntlService } from 'cfx/common/services/intl/intl.service';
import { ScopedLogger } from 'cfx/common/services/log/scopedLogger';
import { IServersBoostService } from 'cfx/common/services/servers/serversBoost.service';
import { IServerBoost, IServerView } from 'cfx/common/services/servers/types';
import { fetcher } from 'cfx/utils/fetcher';

import { MpMenuServersService } from './servers.mpMenu';
import { IDiscourseService } from '../discourse/discourse.service';

export enum BoostUIState {
  Idle,
  Boosting,
  NoAccount,
  Error,
  Success,
}

export function registerMpMenuServersBoostService(container: ServicesContainer) {
  container.registerImpl(IServersBoostService, MpMenuServersBoostService);
}

@injectable()
export class MpMenuServersBoostService implements IServersBoostService {
  private _currentBoost: IServerBoost | null = null;
  public get currentBoost(): IServerBoost | null {
    return this._currentBoost;
  }
  private set currentBoost(currentBoost: IServerBoost | null) {
    this._currentBoost = currentBoost;
  }

  private _currentBoostLoadComplete: boolean = false;
  public get currentBoostLoadComplete(): boolean {
    return this._currentBoostLoadComplete;
  }
  private set currentBoostLoadComplete(currentBoostLoadComplete: boolean) {
    this._currentBoostLoadComplete = currentBoostLoadComplete;
  }

  private _currentBoostLoadError: string = '';
  public get currentBoostLoadError(): string {
    return this._currentBoostLoadError;
  }
  private set currentBoostLoadError(currentBoostLoadError: string) {
    this._currentBoostLoadError = currentBoostLoadError;
  }

  private _uiState: BoostUIState = BoostUIState.Idle;
  public get uiState(): BoostUIState {
    return this._uiState;
  }
  private set uiState(uiState: BoostUIState) {
    this._uiState = uiState;
  }

  private _uiError: string = '';
  public get uiError(): string {
    return this._uiError;
  }
  private set uiError(uiError: string) {
    this._uiError = uiError;
  }

  constructor(
    @inject(ScopedLogger)
    @named('MpMenuServersBoostService')
    protected readonly logService: ScopedLogger,
    @inject(IDiscourseService)
    protected readonly discourseService: IDiscourseService,
    @inject(MpMenuServersService)
    protected readonly serversService: MpMenuServersService,
    @inject(IIntlService)
    protected readonly intlService: IIntlService,
  ) {
    makeAutoObservable(this);

    discourseService.accountChange.addListener(({
      account,
    }) => {
      if (account) {
        this.loadCurrentBoost();
      }
    });

    if (discourseService.account) {
      this.loadCurrentBoost();
    }
  }

  async boostServer(serverId: string): Promise<void> {
    if (!this.discourseService.account) {
      this.uiState = BoostUIState.NoAccount;

      return;
    }

    this.uiState = BoostUIState.Boosting;

    try {
      const response: UpvoteResponse = await this.discourseService.makeExternalCall(
        'https://servers-frontend.fivem.net/api/upvote/',
        'POST',
        {
          address: serverId,
        },
      );

      if ('error' in response) {
        this.uiError = response.error
        || ':( Assigning BOOST™ failed. Please try again later, or contact FiveM support if this issue persists!';
        this.uiState = BoostUIState.Error;

        return;
      }

      this.uiState = BoostUIState.Success;

      await this.loadCurrentBoost();

      const server = this.serversService.getServer(serverId);

      if (!server) {
        return;
      }

      const upvotePower = parseInt(response.power, 10) || 0;
      const burstPower = parseInt(response.burst, 10) || 0;

      const newServer: IServerView = {
        ...server,
        upvotePower: (server.upvotePower || 0) + upvotePower,
        burstPower: (server.burstPower || 0) + burstPower,
      };

      this.serversService.replaceServer(newServer);
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        try {
          this.uiError = (await e.response.json()).error;
        } catch {
          // noop
        }
      }

      if (!this.uiError) {
        // eslint-disable-next-line @stylistic/max-len
        this.uiError = ':( Assigning BOOST™ failed. Please try again later, or contact FiveM support if this issue persists!';
      }

      this.uiState = BoostUIState.Error;
    }
  }

  readonly closeUi = () => {
    this.uiState = BoostUIState.Idle;
    this.uiError = '';
  };

  private async loadCurrentBoost() {
    this.currentBoostLoadError = '';

    if (!this.discourseService.account) {
      this.currentBoostLoadComplete = true;

      return;
    }

    this.currentBoostLoadComplete = false;

    try {
      this.currentBoost = await this.discourseService.makeExternalCall(
        'https://servers-frontend.fivem.net/api/upvote/',
      );
    } catch (e) {
      if (fetcher.HttpError.is(e)) {
        if (e.status >= 500) {
          this.currentBoostLoadError = this.intlService.translate('#Settings_BoostLoadError');
        }

        console.warn('Failed to load current boost', e);
      }

      // That's not an error, in general
      // noop
    }

    this.currentBoostLoadComplete = true;
  }
}

type UpvoteResponse = { success: true; power: string; burst: string } | { error: string };
