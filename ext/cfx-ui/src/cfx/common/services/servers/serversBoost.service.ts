import { defineService, useService } from "../../../base/servicesContainer";
import { IServerBoost } from "./types";

export const IServersBoostService = defineService<IServersBoostService>('ServersBoostServer');
export interface IServersBoostService {
  readonly currentBoost: IServerBoost | null;
  readonly currentBoostLoadComplete: boolean;

  boostServer(serverId: string): void | Promise<void>;
}

export function useServersBoostService(): IServersBoostService {
  return useService(IServersBoostService);
}
