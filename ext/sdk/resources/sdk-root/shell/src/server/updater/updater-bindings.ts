import { interfaces } from "inversify";
import { bindAppContribution } from "server/app/app-contribution";
import { UpdaterService } from "./updater-service";

export const bindUpdater = (container: interfaces.Container) => {
  container.bind(UpdaterService).toSelf().inSingletonScope();
  bindAppContribution(container, UpdaterService);
};
