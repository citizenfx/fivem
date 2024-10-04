import { inject, injectable, named, optional } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { getConnectEndpoits } from 'cfx/base/serverUtils';
import { ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { IAnalyticsService } from 'cfx/common/services/analytics/analytics.service';
import { ElementPlacements, EventActionNames } from 'cfx/common/services/analytics/types';
import { ScopedLogger } from 'cfx/common/services/log/scopedLogger';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServersConnectService } from 'cfx/common/services/servers/serversConnect.service';
import { serverAddress2ServerView } from 'cfx/common/services/servers/transformers';
import { IServerView } from 'cfx/common/services/servers/types';
import { randomArrayItem } from 'cfx/utils/array';
import { fastRandomId } from 'cfx/utils/random';
import { SingleEventEmitter } from 'cfx/utils/singleEventEmitter';
import { RichEvent } from 'cfx/utils/types';

import { ConnectState } from './connect/state';
import { mpMenu } from '../../mpMenu';
import { MpMenuEvents } from '../../mpMenu.events';
import { IConvarService, KnownConvars } from '../convars/convars.service';

export function registerMpMenuServersConnectService(container: ServicesContainer) {
  container.registerImpl(IServersConnectService, MpMenuServersConnectService);
}

export function useMpMenuServersConnectService() {
  return useService(MpMenuServersConnectService);
}

@injectable()
class MpMenuServersConnectService implements IServersConnectService {
  @inject(IAnalyticsService)
  @optional()
  protected readonly analyticksSerivce: IAnalyticsService;

  public get canConnect(): boolean {
    return this._server === null;
  }

  private _state: ConnectState.Any | null = null;
  public get state(): ConnectState.Any | null {
    return this._state;
  }
  private set state(state: ConnectState.Any | null) {
    this._state = state;

    if (state && this.analyticksSerivce && this.server) {
      const analyticsService = this.analyticksSerivce;
      const {
        server,
      } = this;

      queueMicrotask(() => {
        if (state.type === 'connecting' && state.generated) {
          analyticsService.trackEvent({
            action: EventActionNames.CTAOther,
            properties: {
              element_placement: ElementPlacements.ServerConnect,
              text: `Connecting ${server.hostname} (${server.id})`,
              link_url: '',
            },
          });
        }
      });
    }
  }

  private _resolvingServer: boolean = false;
  public get resolvingServer(): boolean {
    return this._resolvingServer;
  }
  private set resolvingServer(resolvingServer: boolean) {
    this._resolvingServer = resolvingServer;
  }

  private _server: IServerView | null = null;
  public get server(): IServerView | null {
    if (!this._server) {
      return null;
    }

    return this.serversService.getServer(this._server.id) || this._server;
  }
  private set server(server: IServerView | null) {
    this._server = server;
  }

  private currentConnectNonce: string | null = null;

  get showServer(): boolean {
    if (this.state?.type === 'failed') {
      // blank 'fault' is usually internal code (including CnL failures)
      if (this.state.extra?.fault === 'cfx' || !this.state.extra?.fault) {
        return false;
      }
    }

    return true;
  }

  get showModal(): boolean {
    if (this.resolvingServer) {
      return true;
    }

    if (this.state) {
      return true;
    }

    return false;
  }

  readonly connectFailed = new SingleEventEmitter<string>();

  constructor(
    @inject(IServersService)
    protected readonly serversService: IServersService,
    @inject(IConvarService)
    protected readonly convarService: IConvarService,
    @inject(ScopedLogger)
    @named('MpMenuServersConnect')
    protected readonly logService: ScopedLogger,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _server: observable.ref,
    });

    mpMenu.onRich(MpMenuEvents.backfillServerInfo, this.backfillServerInfo);

    mpMenu.onRich(MpMenuEvents.connectTo, (event) => this.handleConnectTo(event.hostnameStr, event.connectParams));
    mpMenu.onRich(MpMenuEvents.connecting, () => {
      this.state = ConnectState.connecting();
    });
    mpMenu.onRich(MpMenuEvents.connectStatus, (event) => {
      this.state = ConnectState.status(event.data);
    });
    mpMenu.onRich(MpMenuEvents.connectFailed, (event) => {
      this.handleConnectFailed(event);
    });

    mpMenu.on('connectCard', (event: IConnectCard) => {
      this.state = ConnectState.card(event.data);
    });

    mpMenu.on('connectBuildSwitchRequest', (event: IConnectBuildSwitchRequest) => {
      this.state = ConnectState.buildSwitchRequest(event.data);
    });
    mpMenu.on('connectBuildSwitch', (event: IConnectBuildSwitch) => {
      this.state = ConnectState.buildSwitchInfo(event.data);
    });
  }

  private readonly handleConnectFailed = (event: RichEvent.Payload<typeof MpMenuEvents.connectFailed>) => {
    this.state = ConnectState.failed(event);
    this.connectFailed.emit(event.message);
  };

  private readonly handleConnectTo = (address: string, connectParams = '') => {
    if (connectParams) {
      try {
        const parsed = new URL(`?${connectParams}`, 'http://dummy');

        const truthy = ['true', '1'];

        const forceStreamerMode = parsed.searchParams.get('streamerMode') || 'false';

        if (truthy.includes(forceStreamerMode)) {
          this.convarService.setBoolean(KnownConvars.streamerMode, true);
        }
      } catch (e) {
        // no-op
      }
    }

    this.connectTo(address);
  };

  async connectTo(serverOrAddress: string | IServerView): Promise<void> {
    if (this.currentConnectNonce) {
      console.warn('Already connecting somewhere');

      return;
    }

    this.resolvingServer = true;
    this.server = await this.resolveServer(serverOrAddress);
    this.resolvingServer = false;

    // Set fake connecting state so UI will appear immediately
    if (!this.state) {
      this.state = { type: 'connecting',
        generated: false };
    }

    this.currentConnectNonce = fastRandomId();

    mpMenu.invokeNative(
      'connectTo',
      JSON.stringify([
        this.pickServerConnectEndPoint(
          this.server,
          typeof serverOrAddress === 'string'
            ? serverOrAddress
            : undefined,
        ),
        this.currentConnectNonce,
      ]),
    );
  }

  get canCancel(): boolean {
    if (!this.state) {
      return false;
    }

    if (this.state.type === 'status' && !this.state.cancelable) {
      return false;
    }

    return true;
  }

  readonly cancel = () => {
    if (!this.canCancel) {
      return;
    }

    this.state = null;
    this.server = null;
    this.currentConnectNonce = null;

    mpMenu.invokeNative('cancelDefer');
  };

  private pickServerConnectEndPoint(server: IServerView, address?: string): string {
    if (address) {
      return address;
    }

    const endpoints = getConnectEndpoits(server);

    if (endpoints.manual) {
      return endpoints.manual;
    }

    if (endpoints.provided) {
      return randomArrayItem(endpoints.provided);
    }

    return server.joinId || server.id;
  }

  // TODO: Make it fail when server address is invalid
  private async resolveServer(serverOrAddress: string | IServerView): Promise<IServerView> {
    if (typeof serverOrAddress === 'string') {
      const server = await this.serversService.loadServerLiveData(serverOrAddress);

      return server || serverAddress2ServerView(serverOrAddress);
    }

    return this.serversService.loadServerLiveData(serverOrAddress);
  }

  private readonly backfillServerInfo = async (event: RichEvent.Payload<typeof MpMenuEvents.backfillServerInfo>) => {
    if (this.currentConnectNonce === event.data.nonce) {
      const historyList = this.serversService.getHistoryList();

      if (historyList && this.server) {
        await historyList.addHistoryServer(await historyList.serverView2HistoryServer(this.server, event.data.server));
      }
    }

    this.currentConnectNonce = null;

    mpMenu.invokeNative('backfillDone');
  };
}

interface IConnectBuildSwitchRequest {
  data: ConnectState.DataFor<ConnectState.BuildSwitchRequest>;
}

interface IConnectBuildSwitch {
  data: ConnectState.DataFor<ConnectState.BuildSwitchInfo>;
}

interface IConnectCard {
  data: ConnectState.DataFor<ConnectState.Card>;
}
