import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { UpdaterService } from "./updater-service";
import { UpdaterUtils } from "./updater-utils";
import { bindContributionProvider } from "backend/contribution-provider";
import { UpdaterContribution } from "./updater-contribution";

export const bindUpdater = (container: interfaces.Container) => {
  container.bind(UpdaterUtils).toSelf().inSingletonScope();

  container.bind(UpdaterService).toSelf().inSingletonScope();
  bindAppContribution(container, UpdaterService);

  bindContributionProvider(container, UpdaterContribution);
};
