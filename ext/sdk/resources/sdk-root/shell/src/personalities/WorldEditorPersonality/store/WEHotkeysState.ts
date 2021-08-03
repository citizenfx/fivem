import { makeAutoObservable } from "mobx";
import { getPrintableKeystroke, Keystroke } from "utils/HotkeyController";
import { getCommandBindingKeystroke, getDefaultCommandBindingKeystroke, updateCommandKeystroke } from "../command-bindings";
import { WECommandType } from "../constants/commands";

const LS_KEY = 'we:hotkeys';

function readHotkeys() {
  try {
    const keystrokes = JSON.parse(localStorage[LS_KEY]);

    if (typeof keystrokes === 'object' && !Array.isArray(keystrokes)) {
      return keystrokes;
    }

    return {};
  } catch (e) {
    return {};
  }
}

function saveHotkeys(hotkeys: Record<string, Keystroke>) {
  localStorage[LS_KEY] = JSON.stringify(hotkeys);
}

export const WEHotkeysState = new class WEHotkeysState {
  public hotkeys: Record<string, Keystroke> = readHotkeys();

  constructor() {
    makeAutoObservable(this);

    for (const [command, keystroke] of Object.entries(this.hotkeys)) {
      updateCommandKeystroke(command, keystroke);
    }
  }

  getCommandHotkey(command: WECommandType): string {
    const keystroke = this.hotkeys[command] || getCommandBindingKeystroke(command);
    if (!keystroke) {
      return '';
    }

    return getPrintableKeystroke(keystroke);
  }

  setCommandHotkey(command: WECommandType, keystroke: Keystroke) {
    this.hotkeys[command] = keystroke;

    updateCommandKeystroke(command, keystroke);

    saveHotkeys(this.hotkeys);
  }

  resetCommandHotkey(command: WECommandType) {
    delete this.hotkeys[command];

    const keystroke = getDefaultCommandBindingKeystroke(command);

    if (keystroke) {
      updateCommandKeystroke(command, keystroke);
    }

    saveHotkeys(this.hotkeys);
  }
};
