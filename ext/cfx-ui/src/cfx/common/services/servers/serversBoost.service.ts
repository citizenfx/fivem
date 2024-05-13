import { IServerBoost } from './types';
import { defineService, useService } from '../../../base/servicesContainer';

export const IServersBoostService = defineService<IServersBoostService>('ServersBoostServer');
export interface IServersBoostService {
  readonly currentBoost: IServerBoost | null;
  readonly currentBoostLoadComplete: boolean;
  readonly currentBoostLoadError: string;

  boostServer(serverId: string): void | Promise<void>;
}

export function useServersBoostService(): IServersBoostService {
  return useService(IServersBoostService);
}
