import { IShellCommands } from "./commands";
import { IShellEvents } from "./events";

export interface IShellApi {
  events: IShellEvents,
  commands: IShellCommands,
}

export type IWindowWithShellApi = (typeof window) & {
  shellApi: IShellApi,
};
