import React from 'react';

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
  if (which === 18) {
    return location === 1
      ? 0xA4
      : 0xA5;
  }

  // Ctrl
  if (which === 17) {
    return location === 1
      ? 0xA2
      : 0xA3;
  }

  // Shift
  if (which === 16) {
    return location === 1
      ? 0xA0
      : 0xA1;
  }

  return which;
}

export function isLMB(e: MouseEvent): boolean {
  return e.button === 0;
}

export function isRMB(e: MouseEvent): boolean {
  return e.button === 2;
}

export type ShortcutHandler = (active: boolean, key: number, isCtrl: boolean, isShift: boolean, isAlt: boolean) => void;

export class InputController {
  private readonly activeKeys: Record<number, boolean> = {};
  private readonly activeMouseButtons: Record<number, boolean> = {};

  private cameraControlActive = false;

  private shortcutHandlers: Record<string, ShortcutHandler> = {};

  private inputOverrides = 0;

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

  overrideInput() {
    this.inputOverrides++;

    return () => this.inputOverrides--;
  }

  setKeyboardShortcut(code: string, handler: ShortcutHandler) {
    this.shortcutHandlers[code] = handler;

    return this;
  }

  setActiveKeyboardShortcut(code: string, handler: ShortcutHandler) {
    return this.setKeyboardShortcut(code, (active, ...args) => active && handler(active, ...args));
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
      sendMousePos(event.movementX, event.movementY);

      return;
    }

    const { width, height, x, y } = this.container.current.getBoundingClientRect();

    const rx = (event.x - x) / width;
    const ry = (event.y - y) / height;

    setWorldEditorMouse(rx, ry);
  };

  private handleKeyState(event: KeyboardEvent, active: boolean) {
    if (this.inputOverrides > 0) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();

    const key = mapKey(event.which, event.location);

    this.shortcutHandlers[event.code]?.(active, key, event.ctrlKey, event.shiftKey, event.altKey);

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
    for (const [key, active] of Object.entries(this.activeMouseButtons)) {
      if (active) {
        const button = parseInt(key, 10);

        this.activeMouseButtons[button] = false;
        setMouseButtonState(button, false);
      }
    }
  }
  private handleMouseButtonState(event: MouseEvent, active: boolean) {
    if (this.inputOverrides > 0) {
      return;
    }

    event.preventDefault();
    event.stopPropagation();

    const lmb = isLMB(event);
    const rmb = isRMB(event);
    const button = mapMouseButton(event.button);

    if (this.cameraControlActive) {
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
          setMouseButtonState(button, true);

          this.onSelect(true);

        } else if (!active && this.activeMouseButtons[button]) {
          this.activeMouseButtons[button] = false;
          setMouseButtonState(button, false);

          this.onSelect(false);
        }
      }
    }
  }

  private readonly containerHandlers = {
    mousemove: this.handleContainerMouseMove,
    mousedown: (event: MouseEvent) => this.handleMouseButtonState(event, true),
    mouseup: (event: MouseEvent) => this.handleMouseButtonState(event, false),
  };

  private readonly documentHandlers = {
    keydown: (event: KeyboardEvent) => this.handleKeyState(event, true),
    keyup: (event: KeyboardEvent) => this.handleKeyState(event, false),
  };
}
