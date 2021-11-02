import { registerApiContribution } from "backend/api/api.extensions";
import { ExplorerService } from "./explorer-service";
import { registerSingleton } from "backend/container-access";

registerApiContribution(registerSingleton(ExplorerService));
