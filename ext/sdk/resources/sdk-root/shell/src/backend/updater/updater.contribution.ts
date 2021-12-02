import { registerAppContribution } from "backend/app/app.extensions";
import { UpdaterService } from "./updater-service";
import { UpdaterUtils } from "./updater-utils";
import { registerContributionProvider } from "backend/contribution-provider";
import { registerUpdaterContribution, UpdaterContribution } from "./updater-contribution";
import { UpdaterDummyContribution } from "./updater-dummy-contribution";
import { registerSingleton } from "backend/container-access";

registerSingleton(UpdaterUtils);

registerAppContribution(
  registerSingleton(UpdaterService)
);

registerContributionProvider(UpdaterContribution);

registerUpdaterContribution(
  registerSingleton(UpdaterDummyContribution)
);
