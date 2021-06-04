import { WorldEditorMap, WorldEditorMapVersion } from "./world-editor-types";

export const DEFAULT_WORLD_EDITOR_MAP: WorldEditorMap = {
  version: WorldEditorMapVersion.V1,
  meta: {
    cam: [0, 0, 100, 0, 0, 0],
  },
  patches: {},
  additions: {},
  additionGroups: [],
};
