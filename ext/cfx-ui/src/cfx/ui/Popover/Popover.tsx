import { makeAutoObservable } from 'mobx';
import { observer } from 'mobx-react-lite';
import React from 'react';

import { clsx } from 'cfx/utils/clsx';
import { dispose, Disposer, IDisposableObject } from 'cfx/utils/disposable';
import { useDisposableInstance } from 'cfx/utils/hooks';
import { fastRandomId } from 'cfx/utils/random';

import { Interactive } from '../Interactive/Interactive';

import s from './Popover.module.scss';

const TIMEOUT_ACTIVE = 80;
const TIMEOUT_INACTIVE = 350;

class PopoverController implements IDisposableObject {
  // eslint-disable-next-line no-use-before-define
  static CURRENT_ACTIVE_CONTROLLER: PopoverController | null = null;

  static setActive(ctrl: PopoverController | null) {
    const currentCtrl = PopoverController.CURRENT_ACTIVE_CONTROLLER;

    if (ctrl && currentCtrl && currentCtrl !== ctrl) {
      ctrl.setActive(true, true);
      currentCtrl.setActive(false, true);
    }

    PopoverController.CURRENT_ACTIVE_CONTROLLER = ctrl;
  }

  public wrapperRef: React.RefObject<HTMLDivElement>;

  private _active: boolean = false;
  public get active(): boolean {
    return this._active;
  }

  private set active(active: boolean) {
    this._active = active;

    PopoverController.setActive(active
      ? this
      : null);
  }

  private _mouseWithin: boolean = false;
  public get mouseWithin(): boolean {
    return this._mouseWithin;
  }

  private set mouseWithin(mouseWithin: boolean) {
    this._mouseWithin = mouseWithin;
  }

  private activationTimer: SetTimeoutReturn | null = null;

  private preventDeactivation = false;

  private toDispose = new Disposer();

  private id: string;

  constructor() {
    makeAutoObservable(this, {
      // @ts-expect-error private
      activationTimer: false,
      priventDeactivation: false,
    });

    this.id = fastRandomId();

    if (globalThis.window) {
      window.addEventListener('mouseup', this.handleMouseUp);
      this.toDispose.add(() => window.removeEventListener('mouseup', this.handleMouseUp));
    }
  }

  dispose() {
    dispose(this.toDispose);
  }

  private readonly setActive = (active: boolean, force = false) => {
    if (!active && this.preventDeactivation) {
      return;
    }

    if (this.activationTimer !== null) {
      clearTimeout(this.activationTimer);
    }

    if (force) {
      this._active = active;
    } else {
      this.timedActivate(active);
    }
  };

  private timedActivate(active: boolean) {
    const timeout = active
      ? TIMEOUT_ACTIVE
      : TIMEOUT_INACTIVE;

    this.activationTimer = setTimeout(() => {
      this.activationTimer = null;

      if (!this.active && this.preventDeactivation) {
        return;
      }

      this.active = active;
    }, timeout);
  }

  readonly handleMouseEnter = () => {
    this.setActive(true);
  };

  readonly handleMouseLeave = () => {
    this.setActive(false);
  };

  readonly handleMouseDown = () => {
    this.preventDeactivation = true;
  };

  private readonly handleMouseUp = (event: MouseEvent) => {
    this.preventDeactivation = false;

    if (!this.wrapperRef.current) {
      return;
    }

    if (this.mouseWithin) {
      return;
    }

    if (this.wrapperRef.current.contains(event.target as HTMLElement)) {
      return;
    }

    this.setActive(false);
  };
}

type PopoverPosition = 'top-right';

export interface PopoverProps<T extends HTMLElement> {
  at: PopoverPosition;
  popover: React.ReactNode;
  children: (ref: React.Ref<T>, active: boolean) => React.ReactNode;

  active?: boolean;
}

export const Popover = observer(function Popover<T extends HTMLElement>(props: PopoverProps<T>) {
  const {
    at,
    popover,
    children,
    active: forceActive = false,
  } = props;

  const controller = useDisposableInstance(() => new PopoverController());
  controller.wrapperRef = React.useRef<HTMLDivElement>(null);

  const ref = React.useRef<T>(null);

  const isActive = forceActive || controller.active;

  const rootClassName = clsx(s.root, {
    [s.active]: isActive,
  });

  const popoverClassName = clsx(s.popover, s[`pos-${at}`]);

  return (
    <Interactive
      ref={controller.wrapperRef}
      className={rootClassName}
      onMouseEnter={controller.handleMouseEnter}
      onMouseLeave={controller.handleMouseLeave}
      onMouseDown={controller.handleMouseDown}
    >
      {children(ref, isActive)}

      {isActive && (
        <div className={popoverClassName}>{popover}</div>
      )}
    </Interactive>
  );
});
