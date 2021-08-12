import { KeyboardLayout } from "store/KeyboardLayout";

const codeMap = {
  Escape: 'ESC',
  ShiftLeft: 'Shift',
  ShiftRight: 'Shift',
  AltLeft: 'Alt',
  AltRight: 'Alt',
  ControlLeft: 'Ctrl',
  ControlRight: 'Ctrl',

  ArrowUp: '↑',
  ArrowDown: '↓',
  ArrowLeft: '←',
  ArrowRight: '→',
};

export type HotkeyScope =
  | string
  | number
  | symbol;

export interface Keystroke {
  /**
   * KeyboardEvent.code or KeyboardEvent.key if the former not available (i.e. media keys)
   *
   * @see https://developer.mozilla.org/en-US/docs/Web/API/KeyboardEvent/code
   * @see https://keycode.info/
   */
  code: string,

  alt?: boolean,
  ctrl?: boolean,
  shift?: boolean,
}

export interface HotkeyBinding extends Keystroke {
  command: string,

  /**
   * If not set - global scope
   */
  scope?: HotkeyScope,

  [key: string]: unknown,
}

export type HotkeyTriggeredListener = (binding: HotkeyBinding) => void;

export const SCOPE_GLOBAL = Symbol('SCOPE_GLOBAL');

export class HotkeyController {
  private codeMap: Record<string, HotkeyBinding[]> = {};

  private codeState: Record<string, boolean> = {};
  private modifierState = {
    alt: false,
    ctrl: false,
    shift: false,
  };

  constructor(
    private onTriggered: HotkeyTriggeredListener,
    private bindings: HotkeyBinding[] = [],
    private scope: HotkeyScope = SCOPE_GLOBAL,
  ) {
    this.applyBindings();
  }

  setBingins(bindings: HotkeyBinding[]) {
    this.bindings = bindings;

    this.applyBindings();
  }

  setScope(scope: HotkeyScope) {
    this.scope = scope;
  }

  resetState() {
    this.codeState = {};
    this.modifierState = {
      alt: false,
      ctrl: false,
      shift: false,
    };
  }

  processEvent(event: KeyboardEvent, active: boolean) {
    let retval = false;

    this.applyModifiers(event);

    const code = this.getCode(event);

    if (!active) {
      this.codeState[code] = false;

      return retval;
    }

    if (this.codeState[code]) {
      return retval;
    }
    this.codeState[code] = true;

    const bindings = this.codeMap[code];
    if (!bindings) {
      return retval;
    }

    for (const binding of bindings) {
      if (!this.bindingMatchesScope(binding)) {
        continue;
      }

      if (!this.bindingMatchesModifiers(binding)) {
        continue;
      }

      retval = true;
      this.onTriggered(binding);
    }

    return retval;
  }

  private getCode(event: KeyboardEvent): string {
    return getLowerCase(
      codeMap[event.code] || event.code || event.key,
    );
  }

  private bindingMatchesScope(binding: HotkeyBinding): boolean {
    if (this.scope === SCOPE_GLOBAL) {
      return true;
    }

    if (typeof binding.scope === 'undefined') {
      return true;
    }

    return binding.scope === this.scope;
  }

  private bindingMatchesModifiers(binding: HotkeyBinding): boolean {
    if (!!binding.alt !== this.modifierState.alt) {
      return false;
    }
    if (!!binding.ctrl !== this.modifierState.ctrl) {
      return false;
    }
    if (!!binding.shift !== this.modifierState.shift) {
      return false;
    }

    return true;
  }

  /**
   * Sets modifiers state and returns wether or not has any modifier activated
   */
  private applyModifiers({ altKey, ctrlKey, shiftKey }: KeyboardEvent) {
    this.modifierState.alt = altKey;
    this.modifierState.ctrl = ctrlKey;
    this.modifierState.shift = shiftKey;
  }

  private applyBindings() {
    this.codeMap = {};

    for (const binding of this.bindings) {
      const code = getLowerCase(codeMap[binding.code] || binding.code);

      if (!this.codeMap[code]) {
        this.codeMap[code] = [];
      }

      this.codeMap[code].push(binding);
    }
  }
}

export function getPrintableCode(code: string): string {
  return KeyboardLayout.get(code);
}

export function getPrintableEventKeystroke(event: KeyboardEvent): string {
  return getPrintableKeystroke({
    code: event.code,
    alt: event.altKey,
    ctrl: event.ctrlKey,
    shift: event.shiftKey,
  });
}

export function getEventKeystroke(event: KeyboardEvent): Keystroke | void {
  switch (event.code) {
    case 'ShiftLeft':
    case 'ShiftRight':
    case 'AltLeft':
    case 'AltRight':
    case 'ControlLeft':
    case 'ControlRight': {
      return;
    }
  }

  return {
    code: event.code || event.key,

    alt: event.altKey,
    ctrl: event.ctrlKey,
    shift: event.shiftKey,
  };
}

export function getPrintableKeystroke(keystroke: Keystroke): string {
  const parts = [];

  if (keystroke.alt) {
    parts.push('Alt');
  }
  if (keystroke.ctrl) {
    parts.push('Ctrl');
  }
  if (keystroke.shift) {
    parts.push('Shift');
  }

  parts.push(
    getProperCase(getPrintableCode(keystroke.code) || codeMap[keystroke.code] || keystroke.code),
  );

  return parts.join('+');
}

const _lowerCaseCache = {};
function getLowerCase(str: string): string {
  return _lowerCaseCache[str] || (_lowerCaseCache[str] = str.toLowerCase());
}

const _properCaseCache = {};
function getProperCase(str: string): string {
  return _properCaseCache[str] || (_properCaseCache[str] = str[0].toUpperCase() + str.slice(1));
}
