import { interfaces } from "inversify";
import { bindAppContribution } from "server/app/app-contribution";
import { TheiaService } from "./theia-service";

export const bindTheia = (container: interfaces.Container) => {
  container.bind(TheiaService).toSelf().inSingletonScope();
  bindAppContribution(container, TheiaService);
};
