import { WEMapAddition } from "backend/world-editor/world-editor-types";

export enum HistoryOp {
  ADDITION_CREATED,
  ADDITION_CHANGED,
  ADDITION_DELETED,
}

export interface AdditionCreatedItem {
  op: HistoryOp.ADDITION_CREATED,
  id: string,
  addition: string,
}

export interface AdditionChangedItem {
  op: HistoryOp.ADDITION_CHANGED,
  id: string,
  prev: string,
  next: string,
}

export interface AdditionDeletedItem {
  op: HistoryOp.ADDITION_DELETED,
  id: string,
  addition: string,
}

export type WEHistoryUndoRedoItem =
  | AdditionCreatedItem
  | AdditionChangedItem
  | AdditionDeletedItem
