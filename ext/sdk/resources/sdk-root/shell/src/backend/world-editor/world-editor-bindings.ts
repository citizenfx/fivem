import { bindApiContribution } from "backend/api/api-contribution";
import { interfaces } from "inversify";
import { WorldEditorService } from "./world-editor-service";

export const bindWorldEditor = (container: interfaces.Container) => {
  container.bind(WorldEditorService).toSelf().inSingletonScope();

  bindApiContribution(container, WorldEditorService);
};
