import { defineService } from "../../../base/servicesContainer";
import { IServerView } from "./types";

export const IServersConnectService = defineService<IServersConnectService>('ServersConnectService');
export interface IServersConnectService {
  readonly canConnect: boolean;

  connectTo(server: string | IServerView): void;
}
