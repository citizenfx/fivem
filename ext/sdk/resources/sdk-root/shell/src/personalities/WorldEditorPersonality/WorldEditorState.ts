import React from 'react';
import { makeAutoObservable, runInAction } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { sendApiMessage } from "utils/api";
import { InputController } from "./InputController";
import { onWindowEvent } from 'utils/windowMessages';
import { ShellPersonality, ShellState } from 'store/ShellState';

type WESelection = null | {
  id: number,
  type: number,
  model: number,
};

export enum EditorMode {
  TRANSLATE,
  ROTATE,
  SCALE,
}

export const WorldEditorState = new class WorldEditorState {
  private inputController: InputController;

  public editorSelect = false;
  public editorMode = EditorMode.TRANSLATE;
  public editorLocal = false;

  public selection: WESelection = null;

  constructor() {
    makeAutoObservable(this);

    onWindowEvent('we:selection', (selection: WESelection) => runInAction(() => {
      this.selection = selection;
    }));
  }

  public mapFile: string = 'C:\\dev\\test\\ves\\candy-land.fxworld';
  openMap = (mapFile: string) => {
    this.mapFile = mapFile;

    sendApiMessage(worldEditorApi.start);

    ShellState.setPersonality(ShellPersonality.WORLD_EDITOR);
  };

  closeMap = () => {
    this.mapFile = '';

    sendApiMessage(worldEditorApi.stop);

    ShellState.setPersonality(ShellPersonality.THEIA);
  };

  enableTranslation = () => {
    this.editorMode = EditorMode.TRANSLATE;

    this.updateEditorControls();
  };

  enableRotation = () => {
    this.editorMode = EditorMode.ROTATE;

    this.updateEditorControls();
  };

  enableScaling = () => {
    this.editorMode = EditorMode.SCALE;

    this.updateEditorControls();
  };

  toggleEditorLocal = () => {
    this.editorLocal = !this.editorLocal;

    this.updateEditorControls();
  };

  setEditorSelect = (select: boolean) => {
    this.editorSelect = select;

    this.updateEditorControls();
  };

  createInputController(container: React.RefObject<HTMLDivElement>) {
    this.inputController = new InputController(container, this.setEditorSelect);

    this.inputController
      .setActiveKeyboardShortcut('Digit1', this.enableTranslation)
      .setActiveKeyboardShortcut('Digit2', this.enableRotation)
      .setActiveKeyboardShortcut('Digit3', this.enableScaling)
      .setActiveKeyboardShortcut('Backquote', this.toggleEditorLocal);
  }

  destroyInputController() {
    if (this.inputController) {
      this.inputController.destroy();
      this.inputController = undefined;
    }
  }

  private updateEditorControls() {
    setWorldEditorControls(this.editorSelect, this.editorMode, this.editorLocal);
  }
};
