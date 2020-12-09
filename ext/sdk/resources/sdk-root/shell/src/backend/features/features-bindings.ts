import { interfaces } from "inversify";
import { bindAppContribution } from "backend/app/app-contribution";
import { FeaturesService } from "./features-service";

export const bindFeatures = (container: interfaces.Container) => {
  container.bind(FeaturesService).toSelf().inSingletonScope();
  bindAppContribution(container, FeaturesService);
};
