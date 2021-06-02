//                            posX    posY    posZ    rotX    rotY    rotZ
export type WorldEditorCam = [number, number, number, number, number, number];

export type WorldEditorEntityMatrix = [
  number, number, number, number, // r
  number, number, number, number, // f
  number, number, number, number, // u
  number, number, number, number, // a
];

export interface WorldEditorPatch {
  mat: WorldEditorEntityMatrix,
  cam: WorldEditorCam,
}

export interface WorldEditorApplyPatchRequest {
  patch: WorldEditorPatch,
  mapDataHash: number,
  entityHash: number,
}

export interface WorldEditorMap {
  meta: {
    cam: WorldEditorCam,
  },
  patches: {
    [mapDataHash: number]: {
      [entityHash: number]: WorldEditorPatch,
    },
  },
}
