import { bindApiContribution } from "backend/api/api-contribution";
import { interfaces } from "inversify";
import { WorldEditorArchetypesService } from "./world-editor-archetypes-service";
import { WorldEditorMapCompiler } from "./world-editor-map-compiler";
import { WorldEditorMapUpgrader } from "./world-editor-map-upgrader";
import { WorldEditorService } from "./world-editor-service";

export const bindWorldEditor = (container: interfaces.Container) => {
  container.bind(WorldEditorMapCompiler).toSelf().inSingletonScope();
  container.bind(WorldEditorMapUpgrader).toSelf().inSingletonScope();

  container.bind(WorldEditorService).toSelf().inSingletonScope();

  container.bind(WorldEditorArchetypesService).toSelf().inSingletonScope();

  bindApiContribution(container, WorldEditorService);
};
