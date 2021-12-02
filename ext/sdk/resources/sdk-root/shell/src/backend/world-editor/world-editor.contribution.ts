import { registerApiContribution } from "backend/api/api.extensions";
import { registerSingleton } from "backend/container-access";
import { WorldEditorArchetypesService } from "./world-editor-archetypes-service";
import { WorldEditorMapCompiler } from "./world-editor-map-compiler";
import { WorldEditorMapUpgrader } from "./world-editor-map-upgrader";
import { WorldEditorService } from "./world-editor-service";

registerApiContribution(
  registerSingleton(WorldEditorService)
);

registerSingleton(WorldEditorMapCompiler);
registerSingleton(WorldEditorMapUpgrader);

registerSingleton(WorldEditorArchetypesService);
