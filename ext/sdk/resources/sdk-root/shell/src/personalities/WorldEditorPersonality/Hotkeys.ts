import hotkeys, { HotkeysEvent } from 'hotkeys-js';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { WETool, WorldEditorToolbarState } from './components/WorldEditorToolbar/WorldEditorToolbarState';
import { WEState } from './store/WEState';

export enum HotkeyGroup {
  GAME = 'game',
  CAPTURE = 'capture',
};

export const HOTKEY_COMMAND = {
  TOOL_PATCHES_TOGGLE: 'tool.patches.toggle',
  TOOL_ADDITIONS_TOGGLE: 'tool.additions.toggle',
  TOOL_ADD_OBJECT_TOGGLE: 'tool.addObject.toggle',

  CONTROL_COORD_SYSTEM_TOGGLE: 'control.coordSystem.toggle',
  CONTROL_MODE_TRANSLATE_TOGGLE: 'control.mode.translate.toggle',
  CONTROL_MODE_ROTATE_TOGGLE: 'control.mode.rotate.toggle',
  CONTROL_MODE_SCALE_TOGGLE: 'control.mode.scale.toggle',

  CONTROL_CLEAR_SELECTION: 'control.clearSelection',
};

export interface BeforeHotkeyEvent {
  command: string,
  prevent: boolean,
}

const defaultMapping = {
  [HOTKEY_COMMAND.TOOL_PATCHES_TOGGLE]: 'ctrl+p',
  [HOTKEY_COMMAND.TOOL_ADD_OBJECT_TOGGLE]: 'ctrl+a',
  [HOTKEY_COMMAND.TOOL_ADDITIONS_TOGGLE]: 'ctrl+b',

  [HOTKEY_COMMAND.CONTROL_COORD_SYSTEM_TOGGLE]: '`',
  [HOTKEY_COMMAND.CONTROL_MODE_TRANSLATE_TOGGLE]: '1',
  [HOTKEY_COMMAND.CONTROL_MODE_ROTATE_TOGGLE]: '2',
  [HOTKEY_COMMAND.CONTROL_MODE_SCALE_TOGGLE]: '3',

  [HOTKEY_COMMAND.CONTROL_CLEAR_SELECTION]: 'esc',
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

  private setup() {
    hotkeys.setScope(HotkeyGroup.GAME);

    this.bind(HOTKEY_COMMAND.TOOL_PATCHES_TOGGLE, HotkeyGroup.GAME, () => WorldEditorToolbarState.toggleTool(WETool.Patches));
    this.bind(HOTKEY_COMMAND.TOOL_ADDITIONS_TOGGLE, HotkeyGroup.GAME, () => WorldEditorToolbarState.toggleTool(WETool.Additions));
    this.bind(HOTKEY_COMMAND.TOOL_ADD_OBJECT_TOGGLE, HotkeyGroup.GAME, () => WorldEditorToolbarState.toggleTool(WETool.AddObject));

    this.bind(HOTKEY_COMMAND.CONTROL_COORD_SYSTEM_TOGGLE, HotkeyGroup.GAME, WEState.toggleEditorLocal);
    this.bind(HOTKEY_COMMAND.CONTROL_MODE_TRANSLATE_TOGGLE, HotkeyGroup.GAME, WEState.enableTranslation);
    this.bind(HOTKEY_COMMAND.CONTROL_MODE_ROTATE_TOGGLE, HotkeyGroup.GAME, WEState.enableRotation);
    this.bind(HOTKEY_COMMAND.CONTROL_MODE_SCALE_TOGGLE, HotkeyGroup.GAME, WEState.enableScaling);

    this.bind(HOTKEY_COMMAND.CONTROL_CLEAR_SELECTION, HotkeyGroup.GAME, WEState.clearEditorSelection);
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
