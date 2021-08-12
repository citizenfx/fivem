import { WEMapAddition } from "backend/world-editor/world-editor-types";
import { registerCommandBinding } from "personalities/WorldEditorPersonality/command-bindings";
import { FlashingMessageState } from "personalities/WorldEditorPersonality/components/WorldEditorToolbar/FlashingMessage/FlashingMessageState";
import { WECommandScope } from "personalities/WorldEditorPersonality/constants/commands";
import { UndoRedo } from "shared/undo-redo";
import { WEState } from "../WEState";
import { AdditionChangedItem, HistoryOp, WEHistoryUndoRedoItem } from "./WEHistory.types";

const ADDITION_RECORDING_TIME = 500;

export const WEHistory = new class WEHistory {
  private undoredo: UndoRedo<WEHistoryUndoRedoItem>;

  private additionChangeRecordingTimer: number | null = null;
  private additionChangeRecording: AdditionChangedItem | null = null;

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
    this.commitAdditionChangeRecording();

    this.resetUndoRedo();
  }

  undo() {
    this.commitAdditionChangeRecording();

    const item = this.undoredo.undo();
    if (!item) {
      FlashingMessageState.setMessage('Nothing to undo');
      return;
    }

    switch (item.op) {
      case HistoryOp.ADDITION_CHANGED: {
        WEState.map.setAddition(item.id, JSON.parse(item.prev));

        break;
      }
    }
  }

  redo() {
    this.commitAdditionChangeRecording();

    const item = this.undoredo.redo();
    if (!item) {
      FlashingMessageState.setMessage('Nothing to redo');
      return;
    }

    switch (item.op) {
      case HistoryOp.ADDITION_CHANGED: {
        if (!item.next) {
          return;
        }

        WEState.map.setAddition(item.id, JSON.parse(item.next));

        break;
      }
    }
  }

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

  private resetUndoRedo() {
    this.undoredo = new UndoRedo<WEHistoryUndoRedoItem>(200);
  }
};
