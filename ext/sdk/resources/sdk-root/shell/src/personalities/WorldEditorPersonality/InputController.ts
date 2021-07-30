import React from 'react';
import { clamp01 } from 'shared/math';
import { SingleEventEmitter } from 'utils/singleEventEmitter';

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

export function mapKey(which: number, location: number) {
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
  private cameraMovementBaseMultiplierEvent = new SingleEventEmitter<number>();

  private readonly activeKeys: Record<number, boolean> = {};
  private readonly activeMouseButtons: [boolean, boolean, boolean] = [false, false, false];

  private cameraControlActive = false;
  private cameraMovementBaseMultiplier = 1;

  private inputOverrides = 0;

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
    for (const [event, handler] of Object.entries(this.containerHandlers)) {
      this.container.current.addEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.documentHandlers)) {
      document.addEventListener(event, handler);
    }
  }

  onCameraMovementBaseMultiplierChange(cb: (speed: number) => void) {
    this.cameraMovementBaseMultiplierEvent.addListener(cb);
  }

  overrideInput() {
    this.inputOverrides++;

    return () => this.inputOverrides--;
  }

  deactivateCameraControl() {
    if (this.cameraControlActive) {
      this.cameraControlActive = false;
      document.exitPointerLock();
    }
  }

  destroy() {
    for (const [event, handler] of Object.entries(this.containerHandlers)) {
      this.container.current.removeEventListener(event, handler);
    }

    for (const [event, handler] of Object.entries(this.documentHandlers)) {
      document.removeEventListener(event, handler);
    }
  }

  private handleContainerMouseMove = (event: MouseEvent) => {
    if (this.inputOverrides > 0) {
      return;
    }

    if (this.cameraControlActive) {
      sendGameClientEvent('we:camrot', JSON.stringify([event.movementX, event.movementY]));

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

  private handleKeyState(event: KeyboardEvent, active: boolean) {
    if (this.inputOverrides > 0 || event.ctrlKey) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();

    const key = mapKey(event.which, event.location);

    switch (event.code) {
      case 'KeyW':
      case 'KeyA':
      case 'KeyS':
      case 'KeyD':
      case 'ShiftLeft':
      case 'ShiftRight':
      case 'AltLeft':
      case 'AltRight':
        if (!active && this.activeKeys[key]) {
          this.activeKeys[key] = false;
          setKeyState(key, false);
        } else if (active && !this.activeKeys[key]) {
          this.activeKeys[key] = true;
          setKeyState(key, true);
        }

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
    if (this.inputOverrides > 0) {
      return;
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
        document.exitPointerLock();
        return;
      }

      return;
    }

    switch (true) {
      case rmb && active: {
        this.cameraControlActive = true;
        this.container.current.requestPointerLock();
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

  private readonly containerHandlers = {
    mousemove: this.handleContainerMouseMove,
    mousedown: (event: MouseEvent) => this.handleMouseButtonState(event, true),
    mouseup: (event: MouseEvent) => this.handleMouseButtonState(event, false),
    wheel: this.handleWheel,
  };

  private readonly documentHandlers = {
    keydown: (event: KeyboardEvent) => this.handleKeyState(event, true),
    keyup: (event: KeyboardEvent) => this.handleKeyState(event, false),
  };
}

function haltEvent(event) {
  event.stopPropagation();
  event.preventDefault();
}
