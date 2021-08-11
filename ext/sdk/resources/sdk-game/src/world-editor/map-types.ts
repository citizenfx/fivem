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
  mat: WEEntityMatrix,
  cam: WECam,
}

export type WEMapAdditionGroup = -1 | string;

export interface WEMapAddition {
  label: string,
  mdl: number | string,
  grp: WEMapAdditionGroup,
  mat: WEEntityMatrix,
  cam: WECam,
}

export interface WEAckEnvironmentRequest {
  hours: number,
  minutes: number,
  prevWeather: number,
  nextWeather: number,
}

export enum WESetEnvirnomentType {
  TIME,
  PERSISTENT_WEATHER,
  RANDOM_WEATHER,
}
export type WESetEnvironmentRequest =
  | { type: WESetEnvirnomentType.TIME, hours: number, minutes: number }
  | { type: WESetEnvirnomentType.RANDOM_WEATHER }
  | { type: WESetEnvirnomentType.PERSISTENT_WEATHER, weather: string };

export enum WESelectionType {
  NONE,
  PATCH,
  ADDITION,
}
export type WESelection =
  | { type: WESelectionType.NONE }
  | { type: WESelectionType.PATCH, mapdata: number, entity: number, label: string }
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
  object: WEMapAddition,
  needsPlacement?: boolean,
}

export interface WESetAdditionRequest {
  id: string,
  object: WEMapAddition,
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

export enum WEMapVersion {
  V1 = 1,
  V2 = 2,
  V3 = 3,
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
  additionGroups: Record<string, {
    label: string,
  }>,
  additions: {
    [entityId: string]: WEMapAddition,
  },
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
