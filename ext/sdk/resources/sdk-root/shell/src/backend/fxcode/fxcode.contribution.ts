import { registerAppContribution } from "backend/app/app.extensions";
import { FXCodeService } from "./fxcode-service";
import { FXCodeUpdaterContribution } from "./fxcode-updater-contibution";
import { registerUpdaterContribution } from "backend/updater/updater-contribution";
import { FXCodeImporter } from "./fxcode-importer";
import { registerApiContribution } from "backend/api/api.extensions";
import { registerSingleton } from "backend/container-access";

registerAppContribution(
  registerSingleton(FXCodeService)
);

registerUpdaterContribution(
  registerSingleton(FXCodeUpdaterContribution)
);

registerApiContribution(
  registerAppContribution(
    registerSingleton(FXCodeImporter)
  )
);
