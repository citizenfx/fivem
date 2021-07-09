import React from 'react';
import { makeAutoObservable, runInAction } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { onApiMessage, sendApiMessage } from "utils/api";
import { InputController } from "../InputController";
import { onWindowEvent } from 'utils/windowMessages';
import { ShellPersonality, ShellState } from 'store/ShellState';
import { FilesystemEntry } from 'shared/api.types';
import { FXWORLD_FILE_EXT } from 'assets/fxworld/fxworld-types';
import { WorldEditorStartRequest } from 'shared/api.requests';
import { WEApplyAdditionChangeRequest, WEApplyPatchRequest, WEMap, WESetAdditionRequest } from 'backend/world-editor/world-editor-types';
import { WorldEditorMapState } from './WorldEditorMapState';
import { __DEBUG_MODE_TOGGLES__ } from 'constants/debug-constants';
import { GameState } from 'store/GameState';
import { Hotkeys, HOTKEY_COMMAND } from '../Hotkeys';

const noop = () => {};

function clampExplorerWidth(width: number): number {
  return Math.max(250, Math.min(document.body.offsetWidth / 2, width));
};

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

export const WEState = new class WEState {
  private inputController: InputController;

  public ready = false;

  public editorSelect = false;
  public editorMode = EditorMode.TRANSLATE;
  public editorLocal = false;

  public objectsBrowserOpen = false;

  public selection: WESelection = null;

  public map: WorldEditorMapState | null = null;
  private mapEntry: FilesystemEntry | null = null;

  public mapExplorerOpen = false;
  public mapExplorerWidth: number = clampExplorerWidth(parseInt(localStorage.weExplorerWidth, 10) || 300);

  constructor() {
    makeAutoObservable(this);

    if (__DEBUG_MODE_TOGGLES__.WORLD_EDITOR_UI_ONLY) {
      this.ready = true;
    }

    onWindowEvent('we:selection', (selection: WESelection) => runInAction(() => {
      this.selection = selection;
    }));

    onWindowEvent('we:ready', () => runInAction(() => {
      this.ready = true;

      if (!GameState.archetypesCollectionReady) {
        GameState.refreshArchetypesCollection();
      }
    }));

    onWindowEvent('we:applyPatch', (request: WEApplyPatchRequest) => this.map?.handleApplyPatchRequest(request));
    onWindowEvent('we:setAddition', (request: WESetAdditionRequest) => this.map?.handleSetAdditionRequest(request));
    onWindowEvent('we:applyAdditionChange', (request: WEApplyAdditionChangeRequest) => this.map?.handleApplyAdditionChangeRequest(request));

    onApiMessage(worldEditorApi.mapLoaded, (map: WEMap) => runInAction(() => {
      this.map = new WorldEditorMapState(map);
    }));
  }

  get mapName(): string {
    if (this.mapEntry) {
      return this.mapEntry.name.replace(FXWORLD_FILE_EXT, '');
    }

    return '';
  }

  setCam(cam: number[]) {
    sendGameClientEvent('we:setCam', JSON.stringify(cam));
  }

  openMap = (entry: FilesystemEntry) => {
    this.map = null;
    this.mapEntry = entry;

    sendApiMessage(worldEditorApi.start, {
      mapPath: entry.path,
    } as WorldEditorStartRequest);

    ShellState.setPersonality(ShellPersonality.WORLD_EDITOR);
  };

  closeMap = () => {
    if (false === __DEBUG_MODE_TOGGLES__.WORLD_EDITOR_UI_ONLY) {
      this.ready = false;
    }

    this.map = null;
    this.mapEntry = null;

    sendApiMessage(worldEditorApi.stop);

    ShellState.setPersonality(ShellPersonality.THEIA);
  };

  setExplorerWidth(width: number) {
    this.mapExplorerWidth = clampExplorerWidth(width);

    localStorage.weExplorerWidth = this.mapExplorerWidth;
  }

  toggleObjectsBrowser = () => {
    this.objectsBrowserOpen = !this.objectsBrowserOpen;
  };

  openObjectsBrowser = () => {
    this.objectsBrowserOpen = true;
  };

  closeObjectsBrowser = () => {
    this.objectsBrowserOpen = false;
  };

  toggleMapExplorer = () => {
    this.mapExplorerOpen = !this.mapExplorerOpen;
  };

  openMapExplorer = () => {
    this.mapExplorerOpen = true;
  };

  closeMapExplorer = () => {
    this.mapExplorerOpen = false;
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

  clearEditorSelection = () => {
    if (this.selection) {
      sendGameClientEvent('we:clearSelection', '');
    }
  };

  private hotkeys: Hotkeys | null = null;
  createInputController(container: React.RefObject<HTMLDivElement>) {
    this.hotkeys = new Hotkeys();

    this.inputController = new InputController(container, this.setEditorSelect);

    this.hotkeys.onBeforeHotkey((event) => {
      switch (event.command) {
        case HOTKEY_COMMAND.TOOL_ADDITIONS_TOGGLE:
        case HOTKEY_COMMAND.TOOL_ADD_OBJECT_TOGGLE:
        case HOTKEY_COMMAND.TOOL_PATCHES_TOGGLE: {
          this.inputController.deactivateCameraControl();

          break;
        }
      }
    });
  }

  destroyInputController() {
    if (this.inputController) {
      this.inputController.destroy();
      this.inputController = undefined;
    }

    if (this.hotkeys) {
      this.hotkeys.destroy();
      this.hotkeys = null;
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
