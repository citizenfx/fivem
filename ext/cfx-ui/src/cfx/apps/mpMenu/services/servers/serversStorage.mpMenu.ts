import Dexie, { Table, PromiseExtended } from 'dexie';
import { inject, injectable } from 'inversify';
import { makeAutoObservable, observable } from 'mobx';

import { CurrentGameBrand } from 'cfx/base/gameRuntime';
import { ServicesContainer } from 'cfx/base/servicesContainer';
import { AppContribution, registerAppContribution } from 'cfx/common/services/app/app.extensions';
import { scopedLogger, ScopedLogger } from 'cfx/common/services/log/scopedLogger';
import { IParsedServerAddress, parseServerAddress } from 'cfx/common/services/servers/serverAddressParser';
import { IServersStorageService } from 'cfx/common/services/servers/serversStorage.service';
import { IHistoryServer } from 'cfx/common/services/servers/types';
import { Deferred, retry } from 'cfx/utils/async';

import { mpMenu } from '../../mpMenu';
import { IUiMessageService } from '../uiMessage/uiMessage.service';

(Dexie as any).debug = true;

const DB_NAME = 'HistoryServers';

export function registerMpMenuServersStorageService(container: ServicesContainer) {
  container.registerImpl(IServersStorageService, MpMenuServersStorageService);

  registerAppContribution(container, MpMenuServersStorageService);
}

@injectable()
class MpMenuServersStorageService implements IServersStorageService, AppContribution {
  private _lastServersError: string | null = null;
  public get lastServersError(): string | null {
    return this._lastServersError;
  }
  private set lastServersError(lastServersError: string | null) {
    this._lastServersError = lastServersError;
  }

  private lastServersDeferred = new Deferred<void>();

  readonly lastServersPopulated = this.lastServersDeferred.promise;

  private _lastServers: IHistoryServer[] = [];
  private get lastServers(): IHistoryServer[] {
    return this._lastServers;
  }
  private set lastServers(lastServers: IHistoryServer[]) {
    this._lastServers = lastServers;
  }

  private favoriteServersSequenceDeferred = new Deferred<void>();

  readonly favoriteServersSequencePopulated = this.favoriteServersSequenceDeferred.promise;

  private _favoriteServersSequence: string[] = [];
  public get favoriteServersSequence(): string[] {
    return this._favoriteServersSequence;
  }
  private set favoriteServersSequence(favoriteServersSequence: string[]) {
    this._favoriteServersSequence = favoriteServersSequence;
  }

  private db: Dexie;

  private dbOpen = new Deferred();

  private dbExisted: boolean;

  constructor(
    @scopedLogger('MpMenuServersStorageService')
    protected readonly logger: ScopedLogger,
    @inject(IUiMessageService)
    protected readonly uiMessageService: IUiMessageService,
  ) {
    makeAutoObservable(this, {
      // @ts-expect-error private
      _historyServers: observable.shallow,
    });

    this.openDb();
  }

  init() {
    this.loadFavoriteServers();
    this.loadHistoryServers();
  }

  private loadFavoriteServers() {
    mpMenu.on('getFavorites', ({
      list,
    }: { list: string[] }) => {
      this.favoriteServersSequenceDeferred.resolve();

      if (!Array.isArray(list)) {
        console.warn('Failed to set favorite servers list as event.data.list is not an array');

        return;
      }

      this.favoriteServersSequence = list;
    });

    mpMenu.invokeNative('getFavorites');
  }

  getFavoritesServersSequence(): string[] {
    return this._favoriteServersSequence;
  }

  setFavoriteServers(serverIds: string[]): void {
    mpMenu.invokeNative('saveFavorites', JSON.stringify(serverIds));
  }

  getLastServers(): IHistoryServer[] {
    return this._lastServers;
  }

  async addLastServer(historyServer: IHistoryServer): Promise<void> {
    const table = await this.getDbTable();

    if (!table) {
      return;
    }

    try {
      await wrapDexieErrors(table.put(historyServer));
    } catch (e) {
      console.warn('Failed to update history server', {
        error: e,
        historyServer,
      });

      return;
    }

    await this.loadHistoryServers();

    mpMenu.invokeNative('setLastServers', JSON.stringify(this.lastServers.slice(0, 15).reverse()));
  }

  async clearLastServers(): Promise<void> {
    if (this.lastServers.length === 0) {
      return;
    }

    try {
      const table = await this.getDbTable();

      if (!table) {
        return;
      }

      await wrapDexieErrors(table.bulkDelete(this.lastServers.map(({
        address,
      }) => address)));

      this.lastServers = [];

      mpMenu.invokeNative('setLastServers', '[]');
    } catch (e) {
      console.warn(e);
    }
  }

  private async loadHistoryServers() {
    await this.maybeMigrateToIndexedDB();

    this.lastServers = await this.loadHistoryServersFromIndexedDB();

    this.lastServersDeferred.resolve();
  }

  private async maybeMigrateToIndexedDB() {
    if (!window.localStorage.getItem(DEPRECATED_LS_KEYS.HISTORY_SERVERS)) {
      return;
    }

    const table = await this.getDbTable();

    if (!table) {
      return;
    }

    // Attempt to migrate if no DB existed
    if (!this.dbExisted) {
      const seenAddresses: Record<string, true> = {};

      const historyServersFromLocalStorage = DEPRECATED_getSavedHistoryServersFromLocalStorage().filter(
        (historyServer) => {
          if (seenAddresses[historyServer.address]) {
            return false;
          }

          seenAddresses[historyServer.address] = true;

          return true;
        },
      );

      if (historyServersFromLocalStorage.length) {
        for (const historyServer of historyServersFromLocalStorage) {
          try {
            // eslint-disable-next-line no-await-in-loop
            await table.add(historyServer);
          } catch (e) {
            this.logger.log('The following error occured when trying to migrate history server', historyServer);
            console.warn(e);
          }
        }
      }
    }

    window.localStorage.removeItem(DEPRECATED_LS_KEYS.HISTORY_SERVERS);
  }

  private async loadHistoryServersFromIndexedDB(): Promise<IHistoryServer[]> {
    const table = await this.getDbTable();

    if (!table) {
      return [];
    }

    try {
      return reviveHistoryServers(await retry(3, () => wrapDexieErrors(table.orderBy('time').reverse().toArray())));
    } catch (e) {
      console.warn(e);

      return [];
    }
  }

  private async openDb() {
    try {
      this.dbExisted = await Dexie.exists(DB_NAME);

      this.db = new Dexie(DB_NAME);

      /**
       * Migrations
       *
       * When adding new version DO NOT delete existing definition, define next version instead
       * @see https://dexie.org/docs/Dexie/Dexie.version()
       */
      this.db.version(1).stores({
        list: '&address, time',
      });

      await wrapDexieErrors(this.db.open());
    } catch (e) {
      console.warn('Failed to open IndexedDB', { action: 'Opening the database',
        DB_NAME,
        error: e });

      this.lastServersError = 'Failed to load history servers';

      this.uiMessageService.showInfoMessage(
        [
          '[md]History servers functionality is unavailable right now',
          'Make sure you have some free space on your disk',
          '',
          `__You can continue playing ${CurrentGameBrand}!__`,
        ].join('\n'),
      );
    } finally {
      this.dbOpen.resolve();
    }
  }

  private async getDbTable(): Promise<Table | null> {
    await this.dbOpen.promise;

    if (this.lastServersError) {
      return null;
    }

    return (this.db as any).list;
  }
}

function reviveHistoryServers(entries: Array<Partial<IHistoryServer> | null>): IHistoryServer[] {
  const historyServers: IHistoryServer[] = [];
  const seenHistoryServerAddresses: Record<string, true> = {};

  for (const entry of entries) {
    const result = reviveHistoryServer(entry);

    if (!result) {
      continue;
    }

    const [historyServer, parsedAddress] = result;

    if (seenHistoryServerAddresses[parsedAddress.address]) {
      continue;
    }

    seenHistoryServerAddresses[parsedAddress.address] = true;

    historyServers.push(historyServer);
  }

  return historyServers;
}

function reviveHistoryServer(
  rawHistoryServer: Partial<IHistoryServer> | null,
): [IHistoryServer, IParsedServerAddress] | null {
  if (typeof rawHistoryServer !== 'object' || rawHistoryServer === null) {
    return null;
  }

  const {
    address,
    hostname,
    rawIcon,
    time,
    title,
    token,
    vars,
  } = rawHistoryServer;

  if (!address || typeof address !== 'string') {
    return null;
  }

  const parsedAddress = parseServerAddress(address);

  if (!parsedAddress) {
    return null;
  }

  const historyServer: IHistoryServer = {
    address,
    hostname: typeof hostname === 'string'
      ? hostname
      : address,
    rawIcon: parseIcon(rawIcon),
    time: parseDate(time),
    title: typeof title === 'string'
      ? title
      : '',
    token: typeof token === 'string'
      ? token
      : '',
    vars: typeof vars === 'object' && vars !== null
      ? vars
      : {},
  };

  return [historyServer, parsedAddress];
}

function parseDate(date: unknown): Date {
  if (date instanceof Date) {
    return date;
  }

  if (typeof date !== 'string') {
    return new Date('2013');
  }

  const parsedDate = new Date(date);

  if (Number.isNaN(parsedDate.valueOf())) {
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

/**
 * We've migrated to the IndexedDB as a storage for last connected servers
 *
 * This code only serves migration purposes
 */
namespace DEPRECATED_LS_KEYS {
  export const HISTORY_SERVERS = 'lastServers';
}

/**
 * @deprecated MIGRATION TO IndexedDB SUPPORT CODE
 */
// eslint-disable-next-line camelcase
function DEPRECATED_getSavedHistoryServersFromLocalStorage(): IHistoryServer[] {
  try {
    const rawHistoryServersString = window.localStorage.getItem(DEPRECATED_LS_KEYS.HISTORY_SERVERS);

    if (!rawHistoryServersString) {
      return [];
    }

    const rawHistoryServers = JSON.parse(rawHistoryServersString);

    if (!Array.isArray(rawHistoryServers)) {
      return [];
    }

    return reviveHistoryServers(rawHistoryServers);
  } catch (e) {
    return [];
  }
}

function wrapDexieErrors<T>(op: PromiseExtended<T>): PromiseExtended<T> {
  return op.catch((e) => {
    throw e;
  });
}
