import { inject, injectable, optional } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';
import React from 'react';

import { ISearchTerm } from 'cfx/base/searchTermsParser';
import { useServiceResolver } from 'cfx/base/servicesContainer';
import { ServerListConfigController } from 'cfx/common/services/servers/lists/ServerListConfigController';
import { IParsedServerAddress, parseServerAddress } from 'cfx/common/services/servers/serverAddressParser';
import { IServersService } from 'cfx/common/services/servers/servers.service';
import { IServersConnectService } from 'cfx/common/services/servers/serversConnect.service';
import { IServerView } from 'cfx/common/services/servers/types';
import { IDisposableObject } from 'cfx/utils/disposable';
import { debounce } from 'cfx/utils/execution';
import { useDisposableInstance } from 'cfx/utils/hooks';

@injectable()
export class ServerFiltersWithDirectConnectController implements IDisposableObject {
  public config: ServerListConfigController | undefined;

  private _server: IServerView | null = null;
  public get server(): IServerView | null {
    if (this.parsedAddress?.type === 'join') {
      if (this.serversService.serversListLoading) {
        return null;
      }

      return this.serversService.getServer(this.parsedAddress.address) || null;
    }

    return this._server;
  }
  private set server(server: IServerView | null) {
    this._server = server;
  }

  private _parsedAddress: IParsedServerAddress | null = null;
  public get parsedAddress(): IParsedServerAddress | null {
    return this._parsedAddress;
  }
  private set parsedAddress(parsedAddress: IParsedServerAddress | null) {
    this._parsedAddress = parsedAddress;
  }

  private _loadingServer: boolean = false;
  public get loadingServer(): boolean {
    if (this.parsedAddress?.type === 'join') {
      return this.serversService.serversListLoading;
    }

    return this._loadingServer;
  }
  private set loadingServer(loadingServer: boolean) {
    this._loadingServer = loadingServer;
  }

  private _inputActive: boolean = false;
  public get inputActive(): boolean {
    return this._inputActive;
  }
  private set inputActive(inputActive: boolean) {
    this._inputActive = inputActive;
  }

  readonly setInputActive = (active: boolean) => {
    this.inputActive = active;
  };

  private queryId = 0;

  private nextQueryId() {
    return ++this.queryId;
  }

  constructor(
    @inject(IServersService)
    protected readonly serversService: IServersService,
    @inject(IServersConnectService)
    @optional()
    protected readonly serversConnectService: IServersConnectService | undefined,
  ) {
    makeAutoObservable(this, {
      config: false,

      // @ts-expect-error private
      _server: observable.ref,
    });
  }

  dispose() {
    // just increment the query id counter so loading code can just ignore the result
    this.nextQueryId();
  }

  readonly handleInputKeyDown = (event: React.KeyboardEvent<HTMLInputElement>) => {
    if (event.code === 'Escape') {
      this.config?.setSearchText('');

      return;
    }

    if (event.code !== 'Enter') {
      return;
    }

    if (!this.lastAddress) {
      return;
    }

    if (!this.config) {
      return;
    }

    const connectTo = this.server || this.lastAddress;

    this.config.setSearchText('');
    this.server = null;
    this.parsedAddress = null;
    this.lastAddress = '';

    this.serversConnectService?.connectTo(connectTo);

    event.preventDefault();
  };

  private lastAddress = '';

  readonly setSearchTerms = async (terms: ISearchTerm[]) => {
    if (terms.length === 0) {
      this.server = null;
      this.lastAddress = '';
      this.loadingServer = false;

      return;
    }

    if (terms[0]?.type !== 'address') {
      this.server = null;
      this.lastAddress = '';
      this.loadingServer = false;

      return;
    }

    const address = terms[0].value;

    if (this.lastAddress === address) {
      return;
    }
    this.lastAddress = address;

    this.parsedAddress = parseServerAddress(address);

    if (!this.parsedAddress) {
      this.server = null;
      this.loadingServer = false;

      return;
    }

    // Easy case - only get server from the list
    if (this.parsedAddress.type === 'join') {
      this.server = this.serversService.getServer(this.parsedAddress.address) || null;
      this.loadingServer = false;

      return;
    }

    this.server = null;
    this.loadingServer = true;

    this.doLoadServer(address, this.nextQueryId());
  };

  private readonly doLoadServer = debounce(async (address: string, queryId: number) => {
    try {
      const server = await this.serversService.loadServerLiveData(address);

      if (this.queryId === queryId) {
        this.server = server;
      }
    } catch (e) {
      // noop
    } finally {
      if (this.queryId === queryId) {
        this.loadingServer = false;
      }
    }
  }, 300);
}

function initDirectConnectController(resolver: ReturnType<typeof useServiceResolver>) {
  return resolver(ServerFiltersWithDirectConnectController);
}

export function useServerFiltersWithDirectConnectController() {
  return useDisposableInstance(initDirectConnectController, useServiceResolver());
}
