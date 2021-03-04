import { bindApiContribution } from "backend/api/api-contribution";
import { bindAppContribution } from "backend/app/app-contribution";
import { interfaces } from "inversify";
import { GameService } from "./game-service";

export const bindGame = (container: interfaces.Container) => {
  container.bind(GameService).toSelf().inSingletonScope();

  bindAppContribution(container, GameService);
  bindApiContribution(container, GameService);
};
