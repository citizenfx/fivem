import { WECreateAdditionRequest, WEMapAddition, WEMapPatch, WESelection } from "backend/world-editor/world-editor-types";
import { SingleEventEmitter } from "utils/singleEventEmitter";

export interface AdditionEvent {
  id: string,
  addition: WEMapAddition,
}

export interface PatchEvent {
  mapdata: number,
  entity: number,
  patch: WEMapPatch,
}

export const WEEvents = new class WEEvents {
  public readonly gizmoSelectChanged = new SingleEventEmitter<boolean>();

  public readonly selectionChanged = new SingleEventEmitter<WESelection>();

  public readonly additionCreated = new SingleEventEmitter<WECreateAdditionRequest>();
  public readonly additionChanged = new SingleEventEmitter<AdditionEvent>();
  public readonly additionDeleted = new SingleEventEmitter<AdditionEvent>();

  public readonly patchDeleted = new SingleEventEmitter<PatchEvent>();

  emitAdittionChanged(id: string, addition: WEMapAddition) {
    this.additionChanged.emit({ id, addition });
  }

  emitAdditionDeleted(id: string, addition: WEMapAddition) {
    this.additionDeleted.emit({ id, addition });
  }

  emitPatchDeleted(mapdata: number, entity: number, patch: WEMapPatch) {
    this.patchDeleted.emit({ mapdata, entity, patch });
  }
};
