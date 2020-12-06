import { interfaces } from "inversify";
import { bindAppContribution } from "server/app/app-contribution";
import { FeaturesService } from "./features-service";

export const bindFeatures = (container: interfaces.Container) => {
  container.bind(FeaturesService).toSelf().inSingletonScope();
  bindAppContribution(container, FeaturesService);
};
