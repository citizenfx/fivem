import { AppContribution, registerAppContribution } from "cfx/common/services/app/app.extensions";
import { ServicesContainer } from "cfx/base/servicesContainer";
import { IServersStorageService } from "cfx/common/services/servers/serversStorage.service";
import { IHistoryServer } from "cfx/common/services/servers/types";
import { SingleEventEmitter } from "cfx/utils/singleEventEmitter";
import { injectable } from "inversify";
import { mpMenu } from "../../mpMenu";
import { parseServerAddress } from "cfx/common/services/servers/utils";

namespace LS_KEYS {
  export const HISTORY_SERVERS = 'lastServers';

  export const LAST_SERVER = 'lastServer';
}

export function registerMpMenuServersStorageService(container: ServicesContainer) {
  container.registerImpl(IServersStorageService, MpMenuServersStorageService);

  registerAppContribution(container, MpMenuServersStorageService);
}

@injectable()
class MpMenuServersStorageService implements IServersStorageService, AppContribution {
  private _historyServers = getSavedHistoryServers();
  private _favoriteServers: string[] = [];

  protected favoriteServersEvent = new SingleEventEmitter<string[]>();
  readonly onFavoriteServers = (cb: Parameters<typeof this.favoriteServersEvent.addListener>[0]) => {
    const disposer = this.favoriteServersEvent.addListener(cb);

    if (this._favoriteServers.length) {
      cb(this._favoriteServers);
    }

    return disposer;
  };

  init() {
    mpMenu.on('getFavorites', ({ list }: { list: string[] }) => {
      if (!Array.isArray(list)) {
        console.warn('Failed to set favorite servers list as event.data.list is not an array');
        return;
      }

      this._favoriteServers = list;
      this.favoriteServersEvent.emit(list);
    });

    mpMenu.invokeNative('getFavorites');
  }

  getLastServers(): IHistoryServer[] {
    return this._historyServers;
  }

  setLastServers(historyServers: IHistoryServer[]): void {
    this._historyServers = historyServers;
    const historyServersString = JSON.stringify(historyServers);

    window.localStorage.setItem(LS_KEYS.HISTORY_SERVERS, historyServersString);
    mpMenu.invokeNative('setLastServers', historyServersString);
  }

  setFavoriteServers(serverIds: string[]): void {
    mpMenu.invokeNative('saveFavorites', JSON.stringify(serverIds));
  }
}

function getSavedHistoryServers(): IHistoryServer[] {
  try {
    const rawHistoryServersString = window.localStorage.getItem(LS_KEYS.HISTORY_SERVERS);
    if (!rawHistoryServersString) {
      return [];
    }

    const rawHistoryServers = JSON.parse(rawHistoryServersString);
    if (!Array.isArray(rawHistoryServers)) {
      return [];
    }

    return rawHistoryServers.map(reviveHistoryServer).filter(Boolean) as IHistoryServer[];
  } catch (e) {
    return [];
  }
}

function reviveHistoryServer(rawHistoryServer: Partial<IHistoryServer> | null): IHistoryServer | null {
  if (typeof rawHistoryServer !== 'object' || rawHistoryServer === null) {
    return null;
  }

  const {
    address,
    hostname,
    icon,
    rawIcon,
    time,
    title,
    token,
    vars,
  } = rawHistoryServer;

  if (!address || typeof address !== 'string') {
    return null;
  }

  if (!parseServerAddress(address)) {
    return null;
  }

  const historyServer: IHistoryServer = {
    address: LEGACY_canonicalizeServerAddress(address),
    hostname: typeof hostname === 'string'
      ? hostname
      : address,
    icon: parseIcon(icon),
    rawIcon: parseIcon(rawIcon),
    time: parseDate(time),
    title: typeof title === 'string'
      ? title
      : '',
    token: typeof token === 'string'
      ? token
      : '',
    vars: (typeof vars === 'object' && vars !== null)
      ? vars
      : {},
  };

  return historyServer;
}

function parseDate(date: unknown): Date {
  if (typeof date !== 'string') {
    return new Date('2013');
  }

  const parsedDate = new Date(date);

  if (isNaN(parsedDate.valueOf())) {
    return new Date('2013');
  }

  return parsedDate;
}

function parseIcon(icon: unknown): string {
  if (typeof icon !== 'string') {
    return '';
  }

  if (!icon.startsWith('data:image')) {
    return '';
  }

  return icon;
}

function LEGACY_canonicalizeServerAddress(address: string) {
  return address.replace(/^cfx.re\/join\//, '');
}

