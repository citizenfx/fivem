import { BaseConfigurableServersList } from './lists/BaseConfigurableServersList';
import { FavoriteServersList } from './lists/FavoriteServersList';
import { HistoryServersList } from './lists/HistoryServersList';
import { IServersList, ServersListType } from './lists/types';
import { IAutocompleteIndex, IServerListSource } from './source/types';
import { IPinnedServersConfig, IServerView, ServerViewDetailsLevel } from './types';
import { defineService, useService } from '../../../base/servicesContainer';

export const IServersService = defineService<IServersService>('ServersService');
export interface IServersService {
  readonly serversListLoading: boolean;

  readonly pinnedServers: string[];
  readonly pinnedServersConfig: Readonly<IPinnedServersConfig> | null;

  readonly autocompleteIndex: IAutocompleteIndex | null;

  readonly listTypes: ServersListType[];
  readonly listSource: IServerListSource;

  readonly totalServersCount: number;

  getList(type: ServersListType): IServersList | undefined;
  getAllList(): BaseConfigurableServersList | undefined;
  getHistoryList(): HistoryServersList | undefined;
  getFavoriteList(): FavoriteServersList | undefined;

  getServer(serverId: string): IServerView | undefined;
  getRealServerId(serverId: string): string;
  getAllServerIds(serverId: string): string[];

  isServerLoading(serverId: string, detailsLevel?: ServerViewDetailsLevel): boolean;
  isServerPinned(serverId: string): boolean;

  loadServerLiveData(server: IServerView): Promise<IServerView>;
  loadServerLiveData(address: string): Promise<IServerView | null>;

  loadServerDetailedData(serverId: string): Promise<void>;

  getTagsForServer(server: IServerView): string[];
}

export function useServersService() {
  return useService(IServersService);
}
