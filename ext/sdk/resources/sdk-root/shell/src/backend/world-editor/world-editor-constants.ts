import { WEMap, WEMapVersion } from "./world-editor-types";

export const DEFAULT_WORLD_EDITOR_MAP: WEMap = {
  version: WEMapVersion.V2,
  meta: {
    cam: [0, 0, 100, 0, 0, -45],
  },
  patches: {},
  additions: {},
  additionGroups: {},
};

export const WORLD_EDITOR_MAP_NO_GROUP = -1;
