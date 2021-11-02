import { registerAppContribution } from "backend/app/app.extensions";
import { FeaturesService } from "./features-service";
import { registerSingleton } from "backend/container-access";

registerAppContribution(registerSingleton(FeaturesService));
