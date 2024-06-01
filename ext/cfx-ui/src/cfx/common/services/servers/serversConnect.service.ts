import { SingleEventEmitter } from "cfx/utils/singleEventEmitter";
import { defineService } from "../../../base/servicesContainer";
import { IServerView } from "./types";

export const IServersConnectService = defineService<IServersConnectService>('ServersConnectService');
export interface IServersConnectService {
  readonly canConnect: boolean;
  readonly connectFailed: SingleEventEmitter<string>;

  connectTo(server: string | IServerView): void;
}
