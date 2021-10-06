import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { UpdaterService } from "./updater-service";
import { UpdaterUtils } from "./updater-utils";
import { bindContributionProvider } from "backend/contribution-provider";
import { bindUpdaterContribution, UpdaterContribution } from "./updater-contribution";
import { UpdaterDummyContribution } from "./updater-dummy-contribution";

export const bindUpdater = (container: interfaces.Container) => {
  container.bind(UpdaterUtils).toSelf().inSingletonScope();

  container.bind(UpdaterService).toSelf().inSingletonScope();
  bindAppContribution(container, UpdaterService);

  bindContributionProvider(container, UpdaterContribution);

  container.bind(UpdaterDummyContribution).toSelf().inSingletonScope();
  bindUpdaterContribution(container, UpdaterDummyContribution);
};
