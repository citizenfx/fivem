import { Symbols } from '@cfx-dev/ui-components';
import { injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { GameName } from 'cfx/base/game';
import { CurrentGameName, currentGameNameIs } from 'cfx/base/gameRuntime';
import { defineService, ServicesContainer, useService } from 'cfx/base/servicesContainer';
import { fetcher } from 'cfx/utils/fetcher';
import { html2react } from 'cfx/utils/html2react';

const PLAYER_STATS_FIVEM = 'https://static.cfx.re/runtime/counts.json';
const PLAYER_STATS_REDM = 'https://static.cfx.re/runtime/counts_rdr3.json';

export const IPlatformStatusService = defineService<IPlatformStatusService>('PlatformStatusService');
export type IPlatformStatusService = PlatformStatusService;

export function registerPlatformStatusService(container: ServicesContainer) {
  container.registerImpl(IPlatformStatusService, PlatformStatusService);
}

export function usePlatformStatusService() {
  return useService(IPlatformStatusService);
}

@injectable()
export class PlatformStatusService {
  private _message: string = '';
  public get message(): string {
    return this._message;
  }
  private set message(message: string) {
    this._message = message;
  }

  private _serviceNotice: React.ReactNode = null;
  public get serviceNotice(): React.ReactNode {
    return this._serviceNotice;
  }

  private _statsCurrentPlayers = '';

  private _statsLast24hPeak = '';

  get stats(): { current: string; last24h: string } {
    return {
      current: this._statsCurrentPlayers,
      last24h: this._statsLast24hPeak,
    };
  }

  get hasStats(): boolean {
    return Boolean(this._statsCurrentPlayers) && Boolean(this._statsLast24hPeak);
  }

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _serviceNotice: observable.ref,
    });

    this.fetchServiceNotice();
    this.fetchStats();
  }

  private async fetchServiceNotice() {
    try {
      this.setServiceNotice(await fetcher.text(`https://runtime.fivem.net/notice_${CurrentGameName}.html`));
    } catch (e) {
      this.setServiceNotice('<div class="warning">Could not connect to backend services. Some issues may occur.</div>');
    }
  }

  private async fetchStats() {
    try {
      const response: [number?, number?, number?] = await fetcher.json(
        currentGameNameIs(GameName.RedM)
          ? PLAYER_STATS_REDM
          : PLAYER_STATS_FIVEM,
      );

      if (!Array.isArray(response)) {
        return;
      }

      const [current, _, last24h] = response.map((stat) => {
        if (typeof stat !== 'number') {
          return Symbols.longDash;
        }

        return (Math.floor(stat) / 1000).toFixed(1);
      });

      this.setStats(current, last24h);
    } catch (e) {
      // noop
    }
  }

  private setServiceNotice(notice: string) {
    const reactified = html2react(notice, { removeRelativeLinks: true });

    if (reactified === null) {
      return;
    }

    if (typeof reactified === 'string' && reactified.trim() === '') {
      return;
    }

    this._serviceNotice = reactified;
  }

  private setStats(current: string, last24h: string) {
    this._statsCurrentPlayers = current;
    this._statsLast24hPeak = last24h;
  }
}
