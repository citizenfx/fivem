import { WEMapAddition, WEMapAdditionGroup, WEMapAdditionGroupDefinition, WEMapPatch, WEMapPatchId, WESelectionType } from "backend/world-editor/world-editor-types";
import { registerCommandBinding } from "personalities/WorldEditorPersonality/command-bindings";
import { FlashingMessageState } from "personalities/WorldEditorPersonality/components/WorldEditorToolbar/FlashingMessage/FlashingMessageState";
import { WECommandScope } from "personalities/WorldEditorPersonality/constants/commands";
import { UndoRedo } from "shared/undo-redo";
import { WEState } from "../WEState";
import { AdditionChangedItem, AdditionCreatedItem, AdditionDeletedItem, AdditionGroupChangedItem, AdditionsGroupChangedItem, AdditionsGroupDeletedItem, HistoryOp, PatchChangedItem, PatchCreatedItem, PatchDeletedItem, WEHistoryUndoRedoItem } from "./WEHistory.types";

const ADDITION_RECORDING_TIME = 500;
const PATCH_RECORDING_TIME = 500;

export const WEHistory = new class WEHistory {
  private undoredo: UndoRedo<WEHistoryUndoRedoItem>;

  constructor() {
    this.resetUndoRedo();

    this.bindCommands();
  }

  private bindCommands() {
    registerCommandBinding({
      command: 'undo',
      execute: () => this.undo(),
      scope: WECommandScope.EDITOR,
    }, { code: 'KeyZ', ctrl: true });

    registerCommandBinding({
      command: 'redo',
      execute: () => this.redo(),
      scope: WECommandScope.EDITOR,
    }, { code: 'KeyZ', ctrl: true, shift: true });
  }

  reset() {
    this.commitAllRecordings();

    this.resetUndoRedo();
  }

  undo() {
    this.commitAllRecordings();

    const item = this.undoredo.undo();
    if (!item) {
      FlashingMessageState.setMessage('Nothing to undo');
      return;
    }

    this.processItem(item, false);
  }

  redo() {
    this.commitAllRecordings();

    const item = this.undoredo.redo();
    if (!item) {
      FlashingMessageState.setMessage('Nothing to redo');
      return;
    }

    this.processItem(item, true);
  }

  private processItem(item: WEHistoryUndoRedoItem, redo: boolean) {
    switch (item.op) {
      case HistoryOp.ADDITION_CREATED: {
        if (redo) {
          WEState.map!.setAddition(item.id, JSON.parse(item.addition));
        } else {
          WEState.map!.deleteAddition(item.id, false);
        }

        break;
      }
      case HistoryOp.ADDITION_DELETED: {
        if (redo) {
          WEState.map!.deleteAddition(item.id, false);
        } else {
          WEState.map!.setAddition(item.id, JSON.parse(item.addition));

          if (item.wasSelected) {
            WEState.setEditorSelection({
              type: WESelectionType.ADDITION,
              id: item.id,
            });
          }
        }

        break;
      }
      case HistoryOp.ADDITION_CHANGED: {
        if (redo) {
          if (!item.next) {
            return;
          }

          WEState.map!.setAddition(item.id, JSON.parse(item.next));
        } else {
          WEState.map!.setAddition(item.id, JSON.parse(item.prev));
        }

        break;
      }
      case HistoryOp.ADDITION_GROUP_CHANGED: {
        if (redo) {
          WEState.map!.setAdditionGroup(item.id, item.next, false);
        } else {
          WEState.map!.setAdditionGroup(item.id, item.prev, false);
        }

        break;
      }
      case HistoryOp.ADDITIONS_GROUP_DELETED: {
        if (redo) {
          WEState.map!.deleteAdditionGroup(item.grp, !!item.additions, false);
        } else {
          WEState.map!.restoreAdditionsGroup(
            item.grp,
            item.grpDefinition,
            item.additions
              ? JSON.parse(item.additions)
              : {},
          );
        }

        break;
      }
      case HistoryOp.ADDITIONS_GROUP_CHANGED: {
        if (redo) {
          WEState.map!.setAdditionsGroupDefinition(item.grp, JSON.parse(item.next));
        } else {
          WEState.map!.setAdditionsGroupDefinition(item.grp, JSON.parse(item.prev));
        }

        break;
      }
      case HistoryOp.PATCH_CREATED: {
        const { mapdata, entity } = this.parsePatchId(item.id);

        if (redo) {
          WEState.map!.setPatch(mapdata, entity, JSON.parse(item.patch));
        } else {
          WEState.map!.deletePatch(mapdata, entity, false);
        }

        break;
      }
      case HistoryOp.PATCH_CHANGED: {
        const { mapdata, entity } = this.parsePatchId(item.id);

        if (redo) {
          WEState.map!.setPatch(mapdata, entity, JSON.parse(item.next));
        } else {
          WEState.map!.setPatch(mapdata, entity, JSON.parse(item.prev));
        }

        break;
      }
      case HistoryOp.PATCH_DELETED: {
        const { mapdata, entity } = this.parsePatchId(item.id);

        if (redo) {
          WEState.map!.deletePatch(mapdata, entity, false);
        } else {
          const patch: WEMapPatch = JSON.parse(item.patch);

          WEState.map!.setPatch(mapdata, entity, patch);

          if (item.wasSelected) {
            WEState.setEditorSelection({
              type: WESelectionType.PATCH,
              mapdata,
              entity,
              label: patch.label,
            });
          }
        }

        break;
      }
    }
  }

  patchCreated(mapdata: number, entity: number, patch: WEMapPatch) {
    this.commitAllRecordings();

    this.undoredo.push({
      op: HistoryOp.PATCH_CREATED,
      id: this.getPatchId(mapdata, entity),
      patch: JSON.stringify(patch),
    } as PatchCreatedItem);
  }

  private patchChangeRecordingTimer: number | null = null;
  private patchChangeRecording: PatchChangedItem | null = null;

  beginPatchChange(mapdata: number, entity: number, patch: WEMapPatch) {
    const id = this.getPatchId(mapdata, entity);

    if (this.patchChangeRecording?.id !== id) {
      this.commitPatchChangeRecording();
      this.restartPatchChangeRecordingTimer();

      this.patchChangeRecording = {
        op: HistoryOp.PATCH_CHANGED,
        id,
        prev: JSON.stringify(patch),
        next: '',
      };

      this.undoredo.push(this.patchChangeRecording);
    }
  }
  finishPatchChange(patch: WEMapPatch) {
    if (!this.patchChangeRecording) {
      return;
    }

    this.restartPatchChangeRecordingTimer();

    this.patchChangeRecording.next = JSON.stringify(patch);
  }

  private restartPatchChangeRecordingTimer() {
    if (this.patchChangeRecordingTimer !== null) {
      clearTimeout(this.patchChangeRecordingTimer);
    }

    this.patchChangeRecordingTimer = setTimeout(this.commitPatchChangeRecording, PATCH_RECORDING_TIME) as any;
  }

  private readonly commitPatchChangeRecording = () => {
    if (this.patchChangeRecordingTimer !== null) {
      clearTimeout(this.patchChangeRecordingTimer);
      this.patchChangeRecordingTimer = null;
    }

    this.patchChangeRecording = null;
  };

  patchDeleted(mapdata: number, entity: number, patch: WEMapPatch) {
    this.commitAllRecordings();

    this.undoredo.push({
      op: HistoryOp.PATCH_DELETED,
      id: this.getPatchId(mapdata, entity),
      patch: JSON.stringify(patch),
      wasSelected: WEState.isPatchSelected(mapdata, entity),
    } as PatchDeletedItem);
  }

  additionCreated(id: string, addition: WEMapAddition) {
    this.commitAllRecordings();

    this.undoredo.push({
      op: HistoryOp.ADDITION_CREATED,
      id,
      addition: JSON.stringify(addition),
    } as AdditionCreatedItem);
  }

  additionGroupChanged(id: string, prev: WEMapAdditionGroup, next: WEMapAdditionGroup) {
    this.commitAllRecordings();

    this.undoredo.push({
      op: HistoryOp.ADDITION_GROUP_CHANGED,
      id,
      prev,
      next,
    } as AdditionGroupChangedItem);
  }

  additionsGroupDeleted(grp: string, grpDefinition: WEMapAdditionGroupDefinition, additions?: Record<string, WEMapAddition>) {
    this.commitAllRecordings();

    this.undoredo.push({
      op: HistoryOp.ADDITIONS_GROUP_DELETED,
      grp,
      grpDefinition,
      additions: additions
        ? JSON.stringify(additions)
        : '',
    } as AdditionsGroupDeletedItem);
  }

  additionDeleted(id: string, addition: WEMapAddition) {
    this.commitAllRecordings();

    this.undoredo.push({
      op: HistoryOp.ADDITION_DELETED,
      id,
      addition: JSON.stringify(addition),
      wasSelected: WEState.isAdditionSelected(id),
    } as AdditionDeletedItem);
  }

  additionsGroupChanged(grp: string, prev: WEMapAdditionGroupDefinition, next: WEMapAdditionGroupDefinition) {
    this.undoredo.push({
      op: HistoryOp.ADDITIONS_GROUP_CHANGED,
      grp,
      prev: JSON.stringify(prev),
      next: JSON.stringify(next),
    } as AdditionsGroupChangedItem);
  }

  private additionChangeRecordingTimer: number | null = null;
  private additionChangeRecording: AdditionChangedItem | null = null;

  beginAdditionChange(id: string, addition: WEMapAddition) {
    if (this.additionChangeRecording?.id !== id) {
      this.commitAdditionChangeRecording();
      this.restartAdditionChangeRecordingTimer();

      this.additionChangeRecording = {
        op: HistoryOp.ADDITION_CHANGED,
        id,
        prev: JSON.stringify(addition),
        next: '',
      };

      this.undoredo.push(this.additionChangeRecording);
    }
  }
  finishAdditionChange(addition: WEMapAddition) {
    if (!this.additionChangeRecording) {
      return;
    }

    this.restartAdditionChangeRecordingTimer();

    this.additionChangeRecording.next = JSON.stringify(addition);
  }

  private restartAdditionChangeRecordingTimer() {
    if (this.additionChangeRecordingTimer !== null) {
      clearTimeout(this.additionChangeRecordingTimer);
    }

    this.additionChangeRecordingTimer = setTimeout(this.commitAdditionChangeRecording, ADDITION_RECORDING_TIME) as any;
  }

  private readonly commitAdditionChangeRecording = () => {
    if (this.additionChangeRecordingTimer !== null) {
      clearTimeout(this.additionChangeRecordingTimer);
      this.additionChangeRecordingTimer = null;
    }

    this.additionChangeRecording = null;
  };

  private readonly commitAllRecordings = () => {
    this.commitAdditionChangeRecording();
    this.commitPatchChangeRecording();
  };

  private getPatchId(mapdata: number, entity: number): string {
    return JSON.stringify({
      mapdata,
      entity,
    });
  }

  private parsePatchId(id: string): WEMapPatchId {
    return JSON.parse(id);
  }

  private resetUndoRedo() {
    this.undoredo = new UndoRedo<WEHistoryUndoRedoItem>(200);
  }
}();
