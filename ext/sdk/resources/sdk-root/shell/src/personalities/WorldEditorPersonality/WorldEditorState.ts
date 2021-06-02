import React from 'react';
import { makeAutoObservable, runInAction } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { sendApiMessage } from "utils/api";
import { InputController } from "./InputController";
import { onWindowEvent } from 'utils/windowMessages';
import { ShellPersonality, ShellState } from 'store/ShellState';
import { FilesystemEntry } from 'shared/api.types';
import { FXWORLD_FILE_EXT } from 'assets/fxworld/fxworld-types';
import { WorldEditorStartRequest } from 'shared/api.requests';

const noop = () => {};

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

  public ready = false;

  public editorSelect = false;
  public editorMode = EditorMode.TRANSLATE;
  public editorLocal = false;

  public objectsBrowserOpen = false;

  public selection: WESelection = null;

  private mapEntry: FilesystemEntry | null = null;

  constructor() {
    makeAutoObservable(this);

    onWindowEvent('we:selection', (selection: WESelection) => runInAction(() => {
      this.selection = selection;
    }));

    onWindowEvent('we:ready', () => runInAction(() => {
      this.ready = true;
    }));
  }

  get mapName(): string {
    if (this.mapEntry) {
      return this.mapEntry.name.replace(FXWORLD_FILE_EXT, '');
    }

    return '';
  }

  openMap = (entry: FilesystemEntry) => {
    this.mapEntry = entry;

    sendApiMessage(worldEditorApi.start, {
      mapPath: entry.path,
    } as WorldEditorStartRequest);

    ShellState.setPersonality(ShellPersonality.WORLD_EDITOR);
  };

  closeMap = () => {
    this.ready = false;
    this.mapEntry = null;

    sendApiMessage(worldEditorApi.stop);

    ShellState.setPersonality(ShellPersonality.THEIA);
  };

  toggleObjectsBrowser = () => {
    this.objectsBrowserOpen = !this.objectsBrowserOpen;
  };

  openObjectsBrowser = () => {
    this.objectsBrowserOpen = true;
  };

  closeObjectsBrowser = () => {
    this.objectsBrowserOpen = false;
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
      .setActiveKeyboardShortcut('Backquote', this.toggleEditorLocal)
      .setActiveKeyboardShortcut('KeyA', (_active, _key, isCtrl) => {
        if (isCtrl) {
          this.openObjectsBrowser();
          return true;
        }
      });
  }

  destroyInputController() {
    if (this.inputController) {
      this.inputController.destroy();
      this.inputController = undefined;
    }
  }

  overrideInput() {
    if (!this.inputController) {
      return noop;
    }

    return this.inputController.overrideInput();
  }

  private updateEditorControls() {
    setWorldEditorControls(this.editorSelect, this.editorMode, this.editorLocal);
  }
};
