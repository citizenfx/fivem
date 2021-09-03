import { WEMapAdditionGroup, WEMapAdditionGroupDefinition, WEMapPatchId } from "backend/world-editor/world-editor-types";

export enum HistoryOp {
  PATCH_CREATED,
  PATCH_CHANGED,
  PATCH_DELETED,

  ADDITION_CREATED,
  ADDITION_CHANGED,
  ADDITION_DELETED,
  ADDITION_GROUP_CHANGED,

  ADDITIONS_GROUP_CHANGED,
  ADDITIONS_GROUP_DELETED,
}

export interface AdditionCreatedItem {
  op: HistoryOp.ADDITION_CREATED,
  id: string,
  addition: string, // WEMapAddition json
}

export interface AdditionChangedItem {
  op: HistoryOp.ADDITION_CHANGED,
  id: string,
  prev: string, // WEMapAddition json
  next: string, // WEMapAddition json
}

export interface AdditionGroupChangedItem {
  op: HistoryOp.ADDITION_GROUP_CHANGED,
  id: string,
  prev: WEMapAdditionGroup,
  next: WEMapAdditionGroup,
}

export interface AdditionsGroupChangedItem {
  op: HistoryOp.ADDITIONS_GROUP_CHANGED,
  grp: string,
  prev: string, // WEMapAdditionGroupDefinition json
  next: string, // WEMapAdditionGroupDefinition json
}

export interface AdditionsGroupDeletedItem {
  op: HistoryOp.ADDITIONS_GROUP_DELETED,
  grp: string,
  grpDefinition: WEMapAdditionGroupDefinition,
  additions: string,  // Record<string, WEMapAddition> json
}

export interface AdditionDeletedItem {
  op: HistoryOp.ADDITION_DELETED,
  id: string,
  addition: string, // WEMapAddition json
  wasSelected: boolean,
}

export interface PatchCreatedItem {
  op: HistoryOp.PATCH_CREATED,
  id: string, // WEMapPatchId json
  patch: string, // WEMapPatch json
}

export interface PatchChangedItem {
  op: HistoryOp.PATCH_CHANGED,
  id: string, // WEMapPatchId json
  prev: string, // WEMapPatch json
  next: string, // WEMapPatch json
}

export interface PatchDeletedItem {
  op: HistoryOp.PATCH_DELETED,
  id: string, // WEMapPatchId json
  patch: string, // WEMapPatch json
  wasSelected: boolean,
}

export type WEHistoryUndoRedoItem =
  | PatchCreatedItem
  | PatchChangedItem
  | PatchDeletedItem
  | AdditionCreatedItem
  | AdditionChangedItem
  | AdditionGroupChangedItem
  | AdditionDeletedItem
  | AdditionsGroupChangedItem
  | AdditionsGroupDeletedItem
