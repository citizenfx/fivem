import { registerApiContribution } from "backend/api/api.extensions";
import { ProjectManager } from "./project-manager";
import { ProjectAccess } from "./project-access";
import { ProjectBuilder } from "./project-builder";
import { ProjectUpgrade } from "./project-upgrade";
import { registerSingleton } from "backend/container-access";

registerSingleton(ProjectAccess);
registerSingleton(ProjectUpgrade);

registerApiContribution(
  registerSingleton(ProjectManager)
);

registerApiContribution(
  registerSingleton(ProjectBuilder)
);
