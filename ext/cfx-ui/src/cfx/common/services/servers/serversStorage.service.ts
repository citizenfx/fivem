import { defineService } from 'cfx/base/servicesContainer';

import { IHistoryServer } from './types';

export const IServersStorageService = defineService<IServersStorageService>('erversStorageService');
export interface IServersStorageService {
  readonly lastServersError: string | null;

  readonly favoriteServersSequencePopulated: Promise<void>;
  getFavoritesServersSequence(): string[];
  setFavoriteServers(serverIds: string[]): void;

  readonly lastServersPopulated: Promise<void>;
  getLastServers(): IHistoryServer[];
  addLastServer(historyServer: IHistoryServer): Promise<void>;

  clearLastServers(): Promise<void>;
}
