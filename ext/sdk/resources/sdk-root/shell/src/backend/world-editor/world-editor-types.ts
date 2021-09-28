export enum WEMapVersion {
  V1 = 1,
  V2 = 2,
  V3 = 3,
}

//                   posX    posY    posZ    rotX    rotY    rotZ
export type WECam = [number, number, number, number, number, number];

export enum WEEntityMatrixIndex {
  RX, RY, RZ, RW,
  FX, FY, FZ, FW,
  UX, UY, UZ, UW,
  AX, AY, AZ, AW,
}

export type WEEntityMatrix = [
  number, number, number, number, // right
  number, number, number, number, // forward
  number, number, number, number, // up
  number, number, number, number, // at
];

export interface WEMapPatch {
  label: string,

  /**
   * Transformation matrix
   */
  mat: WEEntityMatrix,

  /**
   * Last editing cam state, updated every time one uses gizmo to edit it
   */
  cam: WECam,
}

export interface WEMapPatchId {
  mapdata: number,
  entity: number,
}

export type WEMapAdditionGroup = -1 | string;
export interface WEMapAdditionGroupDefinition {
  label: string,
}

export interface WEMapAddition {
  label: string,

  /**
   * Addition's model, number type is deprecated
   */
  mdl: number | string,

  /**
   * Addition group
   */
  grp: WEMapAdditionGroup,

  /**
   * Transformation matrix
   */
  mat: WEEntityMatrix,

  /**
   * Last editing cam state, updated every time one uses gizmo to edit it
   */
  cam: WECam,

  /**
   * Visibility distance
   */
  vd?: number | undefined,

  /**
   * User-defined event name triggered when objects gets created
   */
  evcreated?: string | undefined,
  /**
   * User-defined event name triggered when object gets deleted due to that it went out of range
   */
  evdeleted?: string | undefined,
}

export interface WEAckEnvironmentRequest {
  hours: number,
  minutes: number,
  prevWeather: number,
  nextWeather: number,
}

export enum WESetEnvirnomentType {
  TIME,
  FREEZE_TIME,
  PERSISTENT_WEATHER,
  RANDOM_WEATHER,
}
export type WESetEnvironmentRequest =
  | { type: WESetEnvirnomentType.TIME, hours: number, minutes: number }
  | { type: WESetEnvirnomentType.FREEZE_TIME, freeze: boolean }
  | { type: WESetEnvirnomentType.RANDOM_WEATHER }
  | { type: WESetEnvirnomentType.PERSISTENT_WEATHER, weather: string };

export enum WESelectionType {
  NONE,
  PATCH,
  ADDITION,
}
export type WESelection =
  | { type: WESelectionType.NONE }
  | { type: WESelectionType.PATCH, mapdata: number, entity: number, label: string, mat?: WEEntityMatrix, cam?: WECam }
  | { type: WESelectionType.ADDITION, id: string };

export interface WECreatePatchRequest {
  patch: WEMapPatch,
  mapDataHash: number,
  entityHash: number,
}

export interface WEDeletePatchRequest {
  mapDataHash: number,
  entityHash: number,
}

export interface WECreateAdditionRequest {
  id: string,
  addition: WEMapAddition,
}

export interface WESetAdditionRequest {
  id: string,
  addition: WEMapAddition,
}

export type WEApplyAdditionChangeRequest = { id: string } & Partial<WEMapAddition>;

export interface WEApplyPatchChangeRequest {
  mapdata: number,
  entity: number,

  mat?: WEEntityMatrix,
  label?: string,
  cam?: WECam,
}

export interface WESetAdditionGroupRequest {
  additionId: string,
  grp: WEMapAdditionGroup,
}

export interface WECreateAdditionGroupRequest {
  grp: string,
  label: string,
}

export interface WEDeleteAdditionGroupRequest {
  grp: string,
  deleteAdditions: boolean,
}

export interface WESetAdditionGroupNameRequest {
  grp: string,
  label: string,
}

export interface WEDeleteAdditionRequest {
  id: string,
}

export interface WEMap {
  version: WEMapVersion,
  meta: {
    cam: WECam,
  },
  patches: {
    [mapDataHash: number]: {
      [entityHash: number]: WEMapPatch,
    },
  },
  additionGroups: Record<string, WEMapAdditionGroupDefinition>,
  additions: Record<string, WEMapAddition>,
}

export interface WESettings {
  mouseSensetivity: number,
  cameraAboveTheGround: boolean,
  showSelectionBoundingBox: boolean,

  playtestSpawnInVehicle: boolean,
  playtestVehicleName: string,
}

export type WESettingsChangeRequest = Partial<WESettings>;

export interface WEFocusInViewRequest {
  cam: WECam,
  lookAt: [number, number, number],
}
