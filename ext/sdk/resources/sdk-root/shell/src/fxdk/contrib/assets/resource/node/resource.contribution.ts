import { registerApiContribution } from "backend/api/api.extensions";
import { registerSingleton } from "backend/container-access";
import { ProjectFsExtensions } from "fxdk/project/node/projectExtensions";
import { RESOURCE_ENTRY_HANDLE } from "../common/resource.constants";
import { ResourceCreatorService } from "./resourceCreatorService";
import { ResourceFsEntryHandler } from "./resourceFsEntryHandler";

registerApiContribution(
  registerSingleton(ResourceCreatorService)
);

ProjectFsExtensions.register(RESOURCE_ENTRY_HANDLE, ResourceFsEntryHandler);
