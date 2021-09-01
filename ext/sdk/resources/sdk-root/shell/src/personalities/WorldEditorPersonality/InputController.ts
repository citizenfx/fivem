import React from 'react';
import { WEApi } from 'backend/world-editor/world-editor-game-api';
import { clamp01 } from 'shared/math';
import { HotkeyController } from 'utils/HotkeyController';
import { executeCommand, getAllCommandHotkeyBindings, onRegisterCommandBinding } from './command-bindings';
import { FlashingMessageState } from './components/WorldEditorToolbar/FlashingMessage/FlashingMessageState';
import { WECommandScope } from './constants/commands';
import { invokeWEApi } from './we-api-utils';

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

export enum MappedMouseButton {
  LEFT = 0,
  MIDDLE = 2,
  RIGHT = 1,
}

export enum ICState {
  IDLE,
  CLICKING,
  DRAGGING,
  CAMERA_CONTROL,
  FULL_CONTROL,
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

  private state = ICState.IDLE;

  private readonly keyState: Record<number, boolean> = {};
  private readonly mouseButtonState: [boolean, boolean, boolean] = [false, false, false];

  private cameraMovementBaseMultiplier = 1;

  private mousePos: Point = { x: 0, y: 0 };

  private get gizmoPrecisionMode(): boolean {
    return !!this.keyState[Key.ALT_LEFT] || !!this.keyState[Key.ALT_RIGHT];
  }

  private get isFullControl(): boolean {
    return this.state === ICState.FULL_CONTROL;
  }

  private get isCameraControl(): boolean {
    return this.state === ICState.CAMERA_CONTROL;
  }

  private get isClicking(): boolean {
    return this.state === ICState.CLICKING;
  }

  private get isDragging(): boolean {
    return this.state === ICState.DRAGGING;
  }

  constructor(
    private readonly container: React.RefObject<HTMLDivElement>,
    private readonly onGizmoControlActive: (active: boolean) => void,
    private readonly onClick: () => void,
  ) {
    this.hotkeys = new HotkeyController(
      (binding) => {
        if (binding.interrupting) {
          this.exitCameraControl();
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
      this.container.current!.addEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.documentHandlers)) {
      document.addEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.windowHandlers)) {
      window.addEventListener(event, handler);
    }
  }

  destroy() {
    this.removeRegisterCommandBindingListener();

    for (const [event, handler] of Object.entries(this.containerHandlers)) {
      this.container.current!.removeEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.documentHandlers)) {
      document.removeEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.windowHandlers)) {
      window.removeEventListener(event, handler);
    }
  }

  setScope(scope: WECommandScope) {
    this.hotkeys.setScope(scope);
  }

  enterFullControl() {
    this.state = ICState.FULL_CONTROL;

    this.setScope(WECommandScope.PLAYTEST);

    if (!this.lockingPointer) {
      this.lockPointer();
    }

    this.resetMouseButtonStates();
    this.resetKeysState();
  }

  exitFullControl() {
    this.state = ICState.IDLE;

    this.setScope(WECommandScope.EDITOR);

    this.unlockPointer();

    this.resetMouseButtonStates();
    this.resetKeysState();
  }

  private escapeFullControlCallback = (_relative: boolean) => {};
  onEscapeFullControl(cb: (relative: boolean) => void) {
    this.escapeFullControlCallback = cb;
  }

  exitCameraControl() {
    if (this.isCameraControl) {
      this.state = ICState.IDLE;
      this.unlockPointer();

      setWorldEditorMouse(this.mousePos.x, this.mousePos.y);
    }
  }

  private handleContainerMouseMove = (event: MouseEvent) => {
    if (this.isFullControl || this.isCameraControl) {
      return;
    }

    if (this.isClicking) {
      this.state = ICState.DRAGGING;
    }

    let rx: number;
    let ry: number;

    if (this.isDragging) {
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

    for (const [keyString, active] of Object.entries(this.keyState)) {
      if (active) {
        this.keyState[keyString] = false;
        setKeyState(parseInt(keyString, 10), false);
      }
    }
  }

  private handleKeyState(event: KeyboardEvent, active: boolean) {
    if (this.isFullControl) {
      if (event.code === 'Escape') {
        this.escapeFullControlCallback(event.shiftKey);
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

    if (this.keyState[key] !== active) {
      this.keyState[key] = active;

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
    this.mouseButtonState.forEach((active, button) => {
      if (active) {
        this.mouseButtonState[button] = false;
        setMouseButtonState(button, false);
      }
    });
  }
  private handleMouseButtonState(event: MouseEvent, active: boolean) {
    const lmb = isLMB(event);
    const rmb = isRMB(event);
    const button = mapMouseButton(event.button);
    const changed = this.mouseButtonState[button] !== active;

    this.mouseButtonState[button] = active;

    if (active) {
      (document.activeElement as any).blur();
    }

    switch (this.state) {
      case ICState.IDLE: {
        if (!active) {
          break;
        }

        if (lmb) {
          this.state = ICState.CLICKING;
          this.mousePos = this.getEventRelativePos(event);
          this.onGizmoControlActive(true);
        }

        if (rmb) {
          this.state = ICState.CAMERA_CONTROL;
          this.mousePos = this.getEventRelativePos(event);
          setWorldEditorMouse(0, 0);
          this.lockPointer();
        }

        break;
      }

      case ICState.FULL_CONTROL: {
        if (!this.lockingPointer) {
          this.lockPointer();
        } else if (changed) {
          setMouseButtonState(button, active);
        }

        break;
      }

      case ICState.CAMERA_CONTROL: {
        if (rmb && !active) {
          this.exitCameraControl();
        }

        break;
      }

      case ICState.CLICKING: {
        if (lmb && !active) {
          this.state = ICState.IDLE;
          this.onGizmoControlActive(false);
          this.onClick();
        }
      }

      case ICState.DRAGGING: {
        if (lmb && !active) {
          this.state = ICState.IDLE;
          this.onGizmoControlActive(false);
        }

        break;
      }
    }

    haltEvent(event);
  }

  private readonly handleWheel = (event: WheelEvent) => {
    if (this.isFullControl) {
      sendMouseWheel(-event.deltaY);

      return haltEvent(event);
    }

    if (this.isCameraControl) {
      this.cameraMovementBaseMultiplier += -event.deltaY * 0.001;

      if (this.cameraMovementBaseMultiplier < 0.01) {
        this.cameraMovementBaseMultiplier = 0.01;
      }
      if (this.cameraMovementBaseMultiplier === 0.11) {
        this.cameraMovementBaseMultiplier = 0.1;
      }

      invokeWEApi(WEApi.SetCamBaseMultiplier, this.cameraMovementBaseMultiplier);

      FlashingMessageState.setMessage(`Camera speed: ${(this.cameraMovementBaseMultiplier * 100) | 0}%`);
    }
  };

  private getEventRelativePos(event: MouseEvent): Point {
    const { width, height, x, y } = this.container.current!.getBoundingClientRect();

    return {
      x: (event.x - x) / width,
      y: (event.y - y) / height,
    };
  }

  private getScaledMovement(event: MouseEvent): Point {
    const { width, height } = this.container.current!.getBoundingClientRect();

    return {
      x: event.movementX / width,
      y: event.movementY / height,
    };
  }

  private lockingPointer = false;
  private lockPointer() {
    this.lockingPointer = true;
    setRawMouseCapture(true);
    this.container.current!.requestPointerLock();
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

  private readonly handleWindowBlur = () => {
    if (this.lockingPointer) {
      this.unlockPointer();
    }

    this.hotkeys.resetState();

    this.resetKeysState();
    this.resetMouseButtonStates();
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

  private readonly windowHandlers = {
    blur: this.handleWindowBlur,
  };
}

function haltEvent(event) {
  event.stopPropagation();
  event.preventDefault();
}
