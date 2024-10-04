import { makeAutoObservable } from 'mobx';
import React from 'react';

export const NavBarState = new (class NavBarState {
  public readonly outletRef: React.RefObject<HTMLDivElement>;

  private _homeButtonVisible = true;
  get homeButtonVisible(): boolean {
    return this._homeButtonVisible;
  }

  private _forceTransparentNav = false;
  get forceTransparentNav(): boolean {
    return this._forceTransparentNav;
  }

  private _ready = false;
  get ready(): boolean {
    return this._ready;
  }

  constructor() {
    makeAutoObservable(this);

    this.outletRef = React.createRef();
  }

  readonly showHomeButton = () => {
    this._homeButtonVisible = true;
  };

  readonly hideHomeButton = () => {
    this._homeButtonVisible = false;
  };

  readonly setReady = () => {
    this._ready = true;
  };

  readonly setNotReady = () => {
    this._ready = false;
  };

  readonly setForceTransparentNav = () => {
    this._forceTransparentNav = true;
  };

  readonly setForceNormalNav = () => {
    this._forceTransparentNav = false;
  };
})();

export function useHideHomeButton() {
  React.useLayoutEffect(() => {
    NavBarState.hideHomeButton();

    return NavBarState.showHomeButton;
  }, []);
}

export function useForceTransparentNav() {
  React.useLayoutEffect(() => {
    NavBarState.setForceTransparentNav();

    return NavBarState.setForceNormalNav;
  }, []);
}
