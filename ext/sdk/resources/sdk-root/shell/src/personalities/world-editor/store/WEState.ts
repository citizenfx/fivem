import { __DEBUG_MODE_TOGGLES__ } from 'constants/debug-constants';

import React from 'react';
import { makeAutoObservable } from "mobx";
import { worldEditorApi } from "shared/api.events";
import { InputController } from "../InputController";
import { ShellPersonality, ShellState } from 'store/ShellState';
import { FilesystemEntry } from 'shared/api.types';
import { FXWORLD_FILE_EXT } from 'assets/fxworld/fxworld-types';
import { APIRQ } from 'shared/api.requests';
import { WEMap, WESelectionType, WESelection, WECam, WEEntityMatrixIndex, WEEntityMatrix } from 'backend/world-editor/world-editor-types';
import { WEMapState } from './WEMapState';
import { GameState } from 'store/GameState';
import { registerCommandBinding } from '../command-bindings';
import { WECommand, WECommandScope } from '../constants/commands';
import { WEEvents } from './WEEvents';
import { LocalStorageValue } from 'store/generic/LocalStorageValue';
import { WEHistory } from './history/WEHistory';
import { invokeWEApi, onWEApi } from '../we-api-utils';
import { WEApi } from 'backend/world-editor/world-editor-game-api';
import { WEToolbarState } from './WEToolbarState';
import { toNumber } from 'utils/conversion';
import { Api } from 'fxdk/browser/Api';

export enum WEMode {
  EDITOR,
  PLAYTEST,
}

export enum EditorMode {
  TRANSLATE,
  ROTATE,
  SCALE,
}

export const WEState = new class WEState {
  private inputController!: InputController;

  public ready = false;

  public mode = WEMode.EDITOR;

  public editorSelect = false;
  public editorMode = EditorMode.TRANSLATE;
  public editorLocal = false;

  public selection: WESelection = { type: WESelectionType.NONE };

  public map: WEMapState | null = null;
  private mapEntry: FilesystemEntry | null = null;

  private introSeen = new LocalStorageValue({
    key: 'we:introSeen',
    defaultValue: false,
  });

  private forceShowIntro = false;

  get mapName(): string {
    if (this.mapEntry) {
      return this.mapEntry.name.replace(FXWORLD_FILE_EXT, '');
    }

    return '';
  }

  get introFirstTime(): boolean {
    return !this.introSeen.get();
  }

  get showIntro(): boolean {
    if (!this.ready) {
      return false;
    }

    if (!this.map) {
      return false;
    }

    if (!this.introSeen.get()) {
      return true;
    }

    return this.forceShowIntro;
  }

  constructor() {
    makeAutoObservable(this);

    if (__DEBUG_MODE_TOGGLES__.WORLD_EDITOR_UI_ONLY) {
      this.ready = true;
    }

    this.bindCommands();
    this.bindEvents();
  }

  private bindEvents() {
    onWEApi(WEApi.Selection, this.updateSelection);

    onWEApi(WEApi.Ready, this.handleGameReady);

    onWEApi(WEApi.PatchCreate, (request) => this.map?.handleCreatePatchRequest(request));

    onWEApi(WEApi.PatchApplyChange, (request) => this.map?.handleApplyPatchChangeRequest(request));

    onWEApi(WEApi.AdditionPlaced, (request) => this.map?.handleAdditionPlaced(request));
    onWEApi(WEApi.AdditionApplyChange, (request) => this.map?.handleApplyAdditionChangeRequest(request));

    Api.onWithAction(worldEditorApi.mapLoaded, (map: WEMap) => {
      this.map = new WEMapState(map);
    });

    WEEvents.additionDeleted.addListener(({ id }) => {
      if (this.selection.type === WESelectionType.ADDITION && this.selection.id === id) {
        this.clearEditorSelection();
      }
    });

    WEEvents.patchDeleted.addListener(({ mapdata, entity }) => {
      if (this.selection.type === WESelectionType.PATCH && this.selection.mapdata === mapdata && this.selection.entity === entity) {
        this.clearEditorSelection();
      }
    });

    WEEvents.selectionChangeRequest.addListener((selection) => {
      this.setEditorSelection(selection);
    });
  }

  private readonly handleGameReady = () => {
    this.ready = true;

    if (!GameState.archetypesCollectionReady) {
      GameState.refreshArchetypesCollection();
    }
  };

  private bindCommands() {
    registerCommandBinding({
      command: WECommand.CONTROL_COORD_SYSTEM_TOGGLE,
      scope: WECommandScope.EDITOR,
      execute: this.toggleEditorLocal,
    }, { code: 'Backquote' });

    registerCommandBinding({
      command: WECommand.CONTROL_MODE_TRANSLATE_TOGGLE,
      scope: WECommandScope.EDITOR,
      execute: this.enableTranslation,
    }, { code: 'Digit1' });

    registerCommandBinding({
      command: WECommand.CONTROL_MODE_ROTATE_TOGGLE,
      scope: WECommandScope.EDITOR,
      execute: this.enableRotation,
    }, { code: 'Digit2' });

    registerCommandBinding({
      command: WECommand.CONTROL_MODE_SCALE_TOGGLE,
      scope: WECommandScope.EDITOR,
      execute: this.enableScaling,
    }, { code: 'Digit3' });

    registerCommandBinding({
      command: WECommand.CONTROL_ESCAPE,
      scope: WECommandScope.EDITOR,
      execute: this.handleEscape,
    }, { code: 'Esc' });

    registerCommandBinding({
      command: WECommand.ACTION_DELETE_SELECTION,
      scope: WECommandScope.EDITOR,
      execute: this.deleteSelection,
    }, { code: 'Delete' });

    registerCommandBinding({
      command: WECommand.ACTION_DUPLICATE_SELECTION,
      label: 'Duplicate selected addition',
      configurable: true,
      scope: WECommandScope.EDITOR,
      execute: this.duplicateSelection,
    }, { code: 'KeyD', ctrl: true });

    registerCommandBinding({
      command: WECommand.ACTION_SET_ADDITION_ON_GROUND,
      label: 'Set selected addition on ground',
      configurable: true,
      scope: WECommandScope.EDITOR,
      execute: () => {
        if (this.map && this.selection.type === WESelectionType.ADDITION) {
          this.map.setAdditionOnGround(this.selection.id);
        }
      },
    }, { code: 'KeyS', ctrl: true });

    registerCommandBinding({
      command: WECommand.ACTION_ENTER_PLAYTEST_MODE,
      label: 'Enter play mode',
      configurable: true,
      scope: WECommandScope.EDITOR,
      execute: this.enterPlaytestMode,
    }, { code: 'F5' });

    registerCommandBinding({
      command: WECommand.ACTION_FOCUS_SELECTION_IN_VIEW,
      label: 'Focus selection in view',
      configurable: true,
      scope: WECommandScope.EDITOR,
      execute: this.focusSelectionInView,
    }, { code: 'KeyF', ctrl: true, shift: true });
  }

  readonly deleteSelection = () => {
    switch (this.selection.type) {
      case WESelectionType.NONE: {
        return;
      }

      case WESelectionType.PATCH: {
        return this.map?.deletePatch(this.selection.mapdata, this.selection.entity);
      }

      case WESelectionType.ADDITION: {
        return this.map?.deleteAddition(this.selection.id);
      }
    }
  };

  readonly duplicateSelection = () => {
    if (this.selection.type === WESelectionType.ADDITION) {
      this.map?.duplicateAddition(this.selection.id);
    }
  };

  readonly openIntro = () => {
    this.forceShowIntro = true;
  };

  readonly closeIntro = () => {
    if (!this.introSeen.get()) {
      this.introSeen.set(true);
    }

    this.forceShowIntro = false;
  };

  enterIntroScope() {
    this.inputController.setScope(WECommandScope.INTRO);
  }
  exitIntroScope() {
    this.inputController.setScope(WECommandScope.EDITOR);
  }

  readonly enterPlaytestMode = () => {
    if (this.mode === WEMode.PLAYTEST) {
      return;
    }

    invokeWEApi(WEApi.EnterPlaytestMode, undefined);

    this.mode = WEMode.PLAYTEST;
    this.inputController.enterFullControl();
  };

  readonly enterEditorMode = (relative = false) => {
    if (this.mode === WEMode.EDITOR) {
      return;
    }

    invokeWEApi(WEApi.ExitPlaytestMode, relative);

    this.mode = WEMode.EDITOR;
    this.inputController.exitFullControl();
  };

  readonly enterHotkeyConfigurationMode = () => {
    this.inputController.setScope(WECommandScope.CONFIGURATOR);
  };

  readonly exitHotkeyConfigurationMode = () => {
    this.inputController.setScope(WECommandScope.EDITOR);
  };

  setCam(cam: number[]) {
    invokeWEApi(WEApi.SetCam, cam as WECam);
  }

  focusSelectionInView = () => {
    let mat: WEEntityMatrix | void;
    let cam: WECam | void;

    switch (this.selection.type) {
      case WESelectionType.ADDITION: {
        const addition = this.map?.additions[this.selection.id];

        mat = addition?.mat;
        cam = addition?.cam;
        break;
      }

      case WESelectionType.PATCH: {
        const patch = this.map?.patches[this.selection.mapdata]?.[this.selection.entity];
        if (patch) {
          mat = patch.mat;
          cam = patch.cam;
        } else if (this.selection.mat && this.selection.cam) {
          mat = this.selection.mat;
          cam = this.selection.cam;
        }
        break;
      }
    }

    if (cam && mat) {
      this.focusInView(cam, [
        mat[WEEntityMatrixIndex.AX],
        mat[WEEntityMatrixIndex.AY],
        mat[WEEntityMatrixIndex.AZ],
      ]);
    }
  };

  focusInView(cam: WECam, lookAt: [number, number, number]) {
    invokeWEApi(WEApi.FocusInView, {
      cam,
      lookAt,
    });
  }

  openMap = (entry: FilesystemEntry) => {
    this.map = null;
    this.mapEntry = entry;

    WEHistory.reset();

    Api.send(worldEditorApi.start, {
      mapPath: entry.path,
    } as APIRQ.WorldEditorStart);

    ShellState.setPersonality(ShellPersonality.WORLD_EDITOR);
  };

  closeMap = () => {
    if (false === __DEBUG_MODE_TOGGLES__.WORLD_EDITOR_UI_ONLY) {
      this.ready = false;
    }

    this.map = null;
    this.mapEntry = null;

    Api.send(worldEditorApi.stop);

    ShellState.setPersonality(ShellPersonality.MAIN);
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

  setGizmoControlActive = (select: boolean) => {
    this.editorSelect = select;
    WEEvents.gizmoSelectChanged.emit(select);

    this.updateEditorControls();
  };

  isAdditionSelected(additionId: string) {
    if (this.selection.type !== WESelectionType.ADDITION) {
      return false;
    }

    return this.selection.id === additionId;
  }

  isPatchSelected(mapdata: string | number, entity: string | number) {
    if (this.selection.type !== WESelectionType.PATCH) {
      return false;
    }

    return this.selection.mapdata === toNumber(mapdata) && this.selection.entity === toNumber(entity);
  }

  readonly setEditorSelection = (selection: WESelection) => {
    this.updateSelection(selection);

    invokeWEApi(WEApi.Selection, selection);
  };

  readonly handleEscape = () => {
    if (this.selection.type === WESelectionType.NONE) {
      return WEToolbarState.closeAllTools();
    }

    this.clearEditorSelection();
  };

  readonly clearEditorSelection = () => {
    this.setEditorSelection({ type: WESelectionType.NONE });
  };

  createInputController(container: React.RefObject<HTMLDivElement>) {
    this.inputController = new InputController(
      container,
      this.setGizmoControlActive,
      this.selectEntityAtCursor,
    );

    this.inputController.onEscapeFullControl(this.enterEditorMode);
  }

  destroyInputController() {
    this.updateSelection({ type: WESelectionType.NONE });

    if (this.inputController) {
      this.inputController.destroy();
    }
  }

  private selectEntityAtCursor = () => invokeWEApi(WEApi.SelectAtCursor, undefined);

  private updateSelection = (selection: WESelection) => {
    this.selection = selection;
    WEEvents.selectionChanged.emit(selection);
  };

  private updateEditorControls() {
    setWorldEditorControls(this.editorSelect, this.editorMode, this.editorLocal);
  }
}();
