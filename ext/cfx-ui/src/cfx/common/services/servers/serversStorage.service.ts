import { IDisposable } from "cfx/utils/disposable";
import { defineService } from "../../../base/servicesContainer";
import { IHistoryServer } from "./types";

export const IServersStorageService = defineService<IServersStorageService>('erversStorageService');
export interface IServersStorageService {
  onFavoriteServers(cb: (list: string[]) => void): IDisposable;
  setFavoriteServers(serverIds: string[]): void;

  getLastServers(): IHistoryServer[];
  setLastServers(historyServers: IHistoryServer[]): void;

  clearLastServers(): void;
}
