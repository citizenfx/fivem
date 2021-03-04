import { interfaces } from "inversify";
import { bindApiContribution } from "backend/api/api-contribution";
import { ExplorerService } from "./explorer-service";

export const bindExplorer = (container: interfaces.Container) => {
  container.bind(ExplorerService).toSelf().inSingletonScope();
  bindApiContribution(container, ExplorerService);
};
