import hotkeys from 'hotkeys-js';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { WETool, WEToolbarState } from './store/WEToolbarState';
import { WEState } from './store/WEState';
import { WESelectionType } from 'backend/world-editor/world-editor-types';

export enum HotkeyGroup {
  EDITOR = 'editor',
  PLAYTEST = 'playtest',
  CONFIGURATION = 'configuration',
};

export const HOTKEY_COMMAND = {
  TOOL_PATCHES_TOGGLE: 'tool.patches.toggle',
  TOOL_ADDITIONS_TOGGLE: 'tool.additions.toggle',
  TOOL_ADD_OBJECT_TOGGLE: 'tool.addObject.toggle',
  TOOL_ENVIRONMENT_TOGGLE: 'tool.environment.toggle',

  CONTROL_COORD_SYSTEM_TOGGLE: 'control.coordSystem.toggle',
  CONTROL_MODE_TRANSLATE_TOGGLE: 'control.mode.translate.toggle',
  CONTROL_MODE_ROTATE_TOGGLE: 'control.mode.rotate.toggle',
  CONTROL_MODE_SCALE_TOGGLE: 'control.mode.scale.toggle',

  CONTROL_CLEAR_SELECTION: 'control.clearSelection',

  ACTION_SET_ADDITION_ON_GROUND: 'action.setAdditionOnGround',
  ACTION_ENTER_PLAYTEST_MODE: 'action.enterPlaytestMode',
};

export interface BeforeHotkeyEvent {
  command: string,
  prevent: boolean,
}

const defaultMapping = {
  [HOTKEY_COMMAND.TOOL_PATCHES_TOGGLE]: 'ctrl+q',
  [HOTKEY_COMMAND.TOOL_ADD_OBJECT_TOGGLE]: 'ctrl+a',
  [HOTKEY_COMMAND.TOOL_ADDITIONS_TOGGLE]: 'ctrl+w',
  [HOTKEY_COMMAND.TOOL_ENVIRONMENT_TOGGLE]: 'ctrl+e',

  [HOTKEY_COMMAND.CONTROL_COORD_SYSTEM_TOGGLE]: '`',
  [HOTKEY_COMMAND.CONTROL_MODE_TRANSLATE_TOGGLE]: '1',
  [HOTKEY_COMMAND.CONTROL_MODE_ROTATE_TOGGLE]: '2',
  [HOTKEY_COMMAND.CONTROL_MODE_SCALE_TOGGLE]: '3',

  [HOTKEY_COMMAND.CONTROL_CLEAR_SELECTION]: 'esc',

  [HOTKEY_COMMAND.ACTION_SET_ADDITION_ON_GROUND]: 'ctrl+shift+s',

  [HOTKEY_COMMAND.ACTION_ENTER_PLAYTEST_MODE]: 'F5',
};

export class Hotkeys {
  private mapping = defaultMapping;

  private beforeHotkeyEvent = new SingleEventEmitter<BeforeHotkeyEvent>();

  constructor() {
    this.setup();
  }

  destroy() {
    hotkeys.unbind();
  }

  onBeforeHotkey(cb: (event: BeforeHotkeyEvent) => void) {
    this.beforeHotkeyEvent.addListener(cb);
  }

  setGroup(group: HotkeyGroup) {
    hotkeys.setScope(group);
  }

  private setup() {
    hotkeys.setScope(HotkeyGroup.EDITOR);

    this.bind(HOTKEY_COMMAND.TOOL_PATCHES_TOGGLE, HotkeyGroup.EDITOR, () => WEToolbarState.toggleTool(WETool.Patches));
    this.bind(HOTKEY_COMMAND.TOOL_ADDITIONS_TOGGLE, HotkeyGroup.EDITOR, () => WEToolbarState.toggleTool(WETool.Additions));
    this.bind(HOTKEY_COMMAND.TOOL_ADD_OBJECT_TOGGLE, HotkeyGroup.EDITOR, () => WEToolbarState.toggleTool(WETool.AddObject));
    this.bind(HOTKEY_COMMAND.TOOL_ENVIRONMENT_TOGGLE, HotkeyGroup.EDITOR, () => WEToolbarState.toggleTool(WETool.Environment));

    this.bind(HOTKEY_COMMAND.CONTROL_COORD_SYSTEM_TOGGLE, HotkeyGroup.EDITOR, WEState.toggleEditorLocal);
    this.bind(HOTKEY_COMMAND.CONTROL_MODE_TRANSLATE_TOGGLE, HotkeyGroup.EDITOR, WEState.enableTranslation);
    this.bind(HOTKEY_COMMAND.CONTROL_MODE_ROTATE_TOGGLE, HotkeyGroup.EDITOR, WEState.enableRotation);
    this.bind(HOTKEY_COMMAND.CONTROL_MODE_SCALE_TOGGLE, HotkeyGroup.EDITOR, WEState.enableScaling);

    this.bind(HOTKEY_COMMAND.CONTROL_CLEAR_SELECTION, HotkeyGroup.EDITOR, WEState.clearEditorSelection);

    this.bind(HOTKEY_COMMAND.ACTION_SET_ADDITION_ON_GROUND, HotkeyGroup.EDITOR, () => {
      if (WEState.map && WEState.selection.type === WESelectionType.ADDITION) {
        WEState.map.setAdditionOnGround(WEState.selection.id);
      }
    });

    this.bind(HOTKEY_COMMAND.ACTION_ENTER_PLAYTEST_MODE, HotkeyGroup.EDITOR, WEState.enterPlaytestMode);
  }

  private bind(command: string, group: HotkeyGroup, cb: () => void) {
    hotkeys(this.mapping[command], group, () => {
      const beforeHotkeyEvent: BeforeHotkeyEvent = {
        command,
        prevent: false,
      };

      this.beforeHotkeyEvent.emit(beforeHotkeyEvent);

      if (!beforeHotkeyEvent.prevent) {
        cb();
      }
    });
  }

  private unbind(command: string, group: HotkeyGroup) {
    hotkeys.unbind(command, group);
  }
}
