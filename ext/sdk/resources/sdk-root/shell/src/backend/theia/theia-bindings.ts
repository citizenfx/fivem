import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { TheiaService } from "./theia-service";
import { TheiaUpdaterContribution } from "./theia-updater-contribution";
import { bindUpdaterContribution } from "backend/updater/updater-contribution";

export const bindTheia = (container: interfaces.Container) => {
  container.bind(TheiaService).toSelf().inSingletonScope();
  bindAppContribution(container, TheiaService);

  container.bind(TheiaUpdaterContribution).toSelf().inSingletonScope();
  bindUpdaterContribution(container, TheiaUpdaterContribution);
};
