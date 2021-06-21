//                            posX    posY    posZ    rotX    rotY    rotZ
export type WorldEditorCam = [number, number, number, number, number, number];

export type WorldEditorEntityMatrix = [
  number, number, number, number, // right
  number, number, number, number, // forward
  number, number, number, number, // up
  number, number, number, number, // at
];

export interface WorldEditorPatch {
  label: string,
  mat: WorldEditorEntityMatrix,
  cam: WorldEditorCam,
}

export interface WorldEditorMapObject {
  label: string,
  hash: number,
  grp: number,
  mat: WorldEditorEntityMatrix,
  cam: WorldEditorCam,
}

export interface WorldEditorApplyPatchRequest {
  patch: WorldEditorPatch,
  mapDataHash: number,
  entityHash: number,
}

export interface WorldEditorSetAdditionRequest {
  id: string,
  object: WorldEditorMapObject,
}

export type WorldEditorApplyAdditionChangeRequest = { id: string } & Partial<WorldEditorMapObject>;

export interface WorldEditorSetAdditionGroupRequest {
  id: string,
  group: number,
}

export interface WorldEditorSetAdditionGroupNameRequest {
  additionIndex: number,
  name: string,
}

export interface WorldEditorDeleteAdditionRequest {
  id: string,
}

export enum WorldEditorMapVersion {
  V1 = 1,
}

export interface WorldEditorMap {
  version: WorldEditorMapVersion,
  meta: {
    cam: WorldEditorCam,
  },
  patches: {
    [mapDataHash: number]: {
      [entityHash: number]: WorldEditorPatch,
    },
  },
  additionGroups: string[],
  additions: {
    [entityId: string]: WorldEditorMapObject,
  },
}
