import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { bindAppContribution } from "backend/app/app-contribution";
import { GameServerInstaller } from "./game-server-installer";
import { GameServerManagerService } from "./game-server-manager-service";
import { GameServerService } from "./game-server-service";

export const bindGameServer = (container: interfaces.Container) => {
  container.bind(GameServerService).toSelf().inSingletonScope();
  bindAppContribution(container, GameServerService);
  bindApiContribution(container, GameServerService);

  container.bind(GameServerManagerService).toSelf().inSingletonScope();
  bindAppContribution(container, GameServerManagerService);
  bindApiContribution(container, GameServerManagerService);

  container.bind(GameServerInstaller).toSelf().inSingletonScope();
};
