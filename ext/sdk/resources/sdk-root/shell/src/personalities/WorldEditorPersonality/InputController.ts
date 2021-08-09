import React from 'react';
import { clamp01 } from 'shared/math';
import { HotkeyController } from 'utils/HotkeyController';
import { SingleEventEmitter } from 'utils/singleEventEmitter';
import { executeCommand, getAllCommandHotkeyBindings, onRegisterCommandBinding } from './command-bindings';
import { WECommandScope } from './constants/commands';

export enum Key {
  ALT = 18,
  ALT_LEFT = 0xA4,
  ALT_RIGHT = 0xA5,

  CTRL = 17,
  CTRL_LEFT = 0xA2,
  CTRL_RIGHT = 0xA3,

  SHIFT = 16,
  SHIFT_LEFT = 0xA0,
  SHIFT_RIGHT = 0xA1,
}

export enum MouseButton {
  LEFT = 0,
  MIDDLE = 1,
  RIGHT = 2,
}

interface Point {
  x: number,
  y: number,
}

export function mapMouseButton(button: number): number {
  if (button === 2) {
    return 1;
  }

  if (button == 1) {
    return 2;
  }

  return button;
}

export function mapKey({ which, location }: KeyboardEvent) {
  // Alt
  if (which === Key.ALT) {
    return location === 1
      ? Key.ALT_LEFT
      : Key.ALT_RIGHT;
  }

  // Ctrl
  if (which === Key.CTRL) {
    return location === 1
      ? Key.CTRL_LEFT
      : Key.CTRL_RIGHT;
  }

  // Shift
  if (which === Key.SHIFT) {
    return location === 1
      ? Key.SHIFT_LEFT
      : Key.SHIFT_RIGHT;
  }

  return which;
}

export function isLMB(e: MouseEvent): boolean {
  return e.button === MouseButton.LEFT;
}

export function isRMB(e: MouseEvent): boolean {
  return e.button === MouseButton.RIGHT;
}

export class InputController {
  private hotkeys: HotkeyController;
  private removeRegisterCommandBindingListener: Function;

  private cameraMovementBaseMultiplierEvent = new SingleEventEmitter<number>();

  private readonly activeKeys: Record<number, boolean> = {};
  private readonly activeMouseButtons: [boolean, boolean, boolean] = [false, false, false];

  private fullControl = false;

  private cameraControlActive = false;
  private cameraMovementBaseMultiplier = 1;

  private mousePos: Point = { x: 0, y: 0 };

  private get gizmoPrecisionMode(): boolean {
    return !!this.activeKeys[Key.ALT_LEFT] || !!this.activeKeys[Key.ALT_RIGHT];
  }

  private get dragging(): boolean {
    return this.activeMouseButtons[MouseButton.LEFT];
  }

  constructor(
    private readonly container: React.RefObject<HTMLDivElement>,
    private readonly onSelect: (select: boolean) => void,
  ) {
    this.hotkeys = new HotkeyController(
      (binding) => {
        if (binding.interrupting) {
          this.deactivateCameraControl();
        }

        executeCommand(binding.command);
      },
      getAllCommandHotkeyBindings(),
      WECommandScope.EDITOR,
    );

    this.removeRegisterCommandBindingListener = onRegisterCommandBinding(() => {
      this.hotkeys.setBingins(getAllCommandHotkeyBindings());
    });

    for (const [event, handler] of Object.entries(this.containerHandlers)) {
      this.container.current.addEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.documentHandlers)) {
      document.addEventListener(event, handler);
    }
  }

  destroy() {
    this.removeRegisterCommandBindingListener();

    for (const [event, handler] of Object.entries(this.containerHandlers)) {
      this.container.current.removeEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.documentHandlers)) {
      document.removeEventListener(event, handler);
    }
  }

  setScope(scope: WECommandScope) {
    this.hotkeys.setScope(scope);
  }

  enterFullControl() {
    this.fullControl = true;

    this.setScope(WECommandScope.PLAYTEST);

    this.lockPointer();

    this.resetMouseButtonStates();
    this.resetKeysState();
  }

  exitFullControl() {
    this.fullControl = false;

    this.setScope(WECommandScope.EDITOR);

    this.unlockPointer();

    this.resetMouseButtonStates();
    this.resetKeysState();
  }

  private escapeFullControlCallback = () => {};
  onEscapeFullControl(cb: () => void) {
    this.escapeFullControlCallback = cb;
  }

  onCameraMovementBaseMultiplierChange(cb: (speed: number) => void) {
    this.cameraMovementBaseMultiplierEvent.addListener(cb);
  }

  deactivateCameraControl() {
    if (this.cameraControlActive) {
      this.cameraControlActive = false;

      this.unlockPointer();
    }
  }

  private handleContainerMouseMove = (event: MouseEvent) => {
    if (this.fullControl) {
      // sendMousePos(event.movementX, event.movementY);

      return;
    }

    if (this.cameraControlActive) {
      // sendMousePos(event.movementX, event.movementY);
      // sendGameClientEvent('we:camrot', JSON.stringify([event.movementX, event.movementY]));

      return;
    }

    let rx: number;
    let ry: number;

    if (this.dragging) {
      const movement = this.getScaledMovement(event);

      const multiplier = this.gizmoPrecisionMode
        ? 0.1
        : 1;

      rx = this.mousePos.x = clamp01(this.mousePos.x + (movement.x * multiplier));
      ry = this.mousePos.y = clamp01(this.mousePos.y + (movement.y * multiplier));
    } else {
      ({ x: rx, y: ry } = this.getEventRelativePos(event));
    }

    setWorldEditorMouse(rx, ry);
  };

  private resetKeysState() {
    this.hotkeys.resetState();

    for (const [keyString, active] of Object.entries(this.activeKeys)) {
      if (active) {
        this.activeKeys[keyString] = false;
        setKeyState(parseInt(keyString, 10), false);
      }
    }
  }

  private handleKeyState(event: KeyboardEvent, active: boolean) {
    if (this.fullControl) {
      if (event.code === 'Escape') {
        this.escapeFullControlCallback();
      } else {
        this.applyKeyState(event, active);
      }

      return haltEvent(event);
    }

    // If any of these in focus - ignore
    const { tagName } = (event.target as any);
    if (tagName === 'INPUT' || tagName === 'TEXTAREA' || tagName === 'SELECT') {
      return;
    }

    if (this.hotkeys.processEvent(event, active)) {
      haltEvent(event);
    } else {
      this.applyGameInput(event, active);
    }
  }

  private applyKeyState(event: KeyboardEvent, active: boolean) {
    const key = mapKey(event);

    if (this.activeKeys[key] !== active) {
      this.activeKeys[key] = active;

      setKeyState(key, active);
    }
  }

  private applyGameInput(event: KeyboardEvent, active: boolean) {
    switch (event.code) {
      case 'KeyW':
      case 'KeyA':
      case 'KeyS':
      case 'KeyD':
      case 'ShiftLeft':
      case 'ShiftRight':
      case 'AltLeft':
      case 'AltRight':
        haltEvent(event);
        this.applyKeyState(event, active);

        break;
    }
  }

  private resetMouseButtonStates() {
    this.activeMouseButtons.forEach((active, button) => {
      if (active) {
        this.activeMouseButtons[button] = false;
        setMouseButtonState(button, false);
      }
    });
  }
  private handleMouseButtonState(event: MouseEvent, active: boolean) {
    if (this.fullControl) {
      const button = mapMouseButton(event.button);

      if (this.activeMouseButtons[button] !== active) {
        this.activeMouseButtons[button] = active;

        setMouseButtonState(button, active);
      }

      return haltEvent(event);
    }

    if (active) {
      (document.activeElement as any).blur();
    }

    const lmb = isLMB(event);
    const rmb = isRMB(event);
    const button = mapMouseButton(event.button);

    if (this.cameraControlActive) {
      haltEvent(event);

      if (rmb && !active) {
        this.cameraControlActive = false;
        this.unlockPointer();
        return;
      }

      return;
    }

    switch (true) {
      case rmb && active: {
        this.cameraControlActive = true;
        this.lockPointer();
        this.resetMouseButtonStates();
        break;
      }

      case lmb: {
        if (active && !this.activeMouseButtons[button]) {
          this.activeMouseButtons[button] = true;
          this.mousePos = this.getEventRelativePos(event);
          setMouseButtonState(button, true);

          this.onSelect(true);

        } else if (!active && this.activeMouseButtons[button]) {
          this.activeMouseButtons[button] = false;
          setMouseButtonState(button, false);

          this.onSelect(false);
        }

        break;
      }

      default: return;
    }

    haltEvent(event);
  }

  private readonly handleWheel = (event: WheelEvent) => {
    if (this.fullControl) {
      sendMouseWheel(-event.deltaY);

      return haltEvent(event);
    }

    if (this.cameraControlActive) {
      this.cameraMovementBaseMultiplier += -event.deltaY * 0.001;

      if (this.cameraMovementBaseMultiplier < 0.01) {
        this.cameraMovementBaseMultiplier = 0.01;
      }
      if (this.cameraMovementBaseMultiplier === 0.11) {
        this.cameraMovementBaseMultiplier = 0.1;
      }

      this.cameraMovementBaseMultiplierEvent.emit(this.cameraMovementBaseMultiplier);

      sendGameClientEvent('we:setCamBaseMultiplier', JSON.stringify(this.cameraMovementBaseMultiplier));
    }
  };

  private getEventRelativePos(event: MouseEvent): Point {
    const { width, height, x, y } = this.container.current.getBoundingClientRect();

    return {
      x: (event.x - x) / width,
      y: (event.y - y) / height,
    };
  }

  private getScaledMovement(event: MouseEvent): Point {
    const { width, height } = this.container.current.getBoundingClientRect();

    return {
      x: event.movementX / width,
      y: event.movementY / height,
    };
  }

  private lockingPointer = false;
  private lockPointer() {
    this.lockingPointer = true;
    setRawMouseCapture(true);
    this.container.current.requestPointerLock();
  }

  private unlockPointer() {
    this.lockingPointer = false;
    setRawMouseCapture(false);
    document.exitPointerLock();
  }

  private readonly handlePointerLockChanged = () => {
    if (this.lockingPointer && !document.pointerLockElement) {
      this.lockingPointer = false;
      setRawMouseCapture(false);
    }
  };

  private readonly containerHandlers = {
    mousemove: this.handleContainerMouseMove,
    mousedown: (event: MouseEvent) => this.handleMouseButtonState(event, true),
    mouseup: (event: MouseEvent) => this.handleMouseButtonState(event, false),
    wheel: this.handleWheel,
  };

  private readonly documentHandlers = {
    pointerlockchange: this.handlePointerLockChanged,
    keydown: (event: KeyboardEvent) => this.handleKeyState(event, true),
    keyup: (event: KeyboardEvent) => this.handleKeyState(event, false),
  };
}

function haltEvent(event) {
  event.stopPropagation();
  event.preventDefault();
}
