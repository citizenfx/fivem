import { registerApiContribution } from "backend/api/api.extensions";
import { registerAppContribution } from "backend/app/app.extensions";
import { registerSingleton } from "backend/container-access";
import { GameConnectionService } from "./game-connection-service";
import { GameService } from "./game-service";

registerApiContribution(
  registerAppContribution(
    registerSingleton(GameService)
  )
);

registerAppContribution(
  registerSingleton(GameConnectionService)
);
