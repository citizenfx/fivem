import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { FXCodeService } from "./fxcode-service";
import { FXCodeUpdaterContribution } from "./fxcode-updater-contibution";
import { bindUpdaterContribution } from "backend/updater/updater-contribution";
import { FXCodeImporter } from "./fxcode-importer";
import { bindApiContribution } from "backend/api/api-contribution";

export const bindFXCode = (container: interfaces.Container) => {
  container.bind(FXCodeService).toSelf().inSingletonScope();
  bindAppContribution(container, FXCodeService);

  container.bind(FXCodeUpdaterContribution).toSelf().inSingletonScope();
  bindUpdaterContribution(container, FXCodeUpdaterContribution);

  container.bind(FXCodeImporter).toSelf().inSingletonScope();
  bindAppContribution(container, FXCodeImporter);
  bindApiContribution(container, FXCodeImporter);
};
