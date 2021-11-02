import { registerApiContribution } from "backend/api/api.extensions";
import { registerAppContribution } from "backend/app/app.extensions";
import { GameServerInstallerUtils } from "./game-server-installer-utils";
import { GameServerManagerService } from "./game-server-manager-service";
import { GameServerService } from "./game-server-service";
import { GameServerRuntime } from "./game-server-runtime";
import { registerSingleton } from "backend/container-access";

registerSingleton(GameServerRuntime);

registerApiContribution(
  registerAppContribution(
    registerSingleton(GameServerService)
  )
);

registerApiContribution(
  registerAppContribution(
    registerSingleton(GameServerManagerService)
  )
);

registerSingleton(GameServerInstallerUtils);
