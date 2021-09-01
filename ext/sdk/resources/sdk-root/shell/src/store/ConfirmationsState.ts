import { makeAutoObservable } from "mobx";
import React from "react";

export interface ConfirmationRequest<State extends object = {}> {
  title: string,
  buttonIcon: React.ReactNode,
  buttonText: string,

  state?: State,

  children?(state: State | null, setState: (newState: State) => void): React.ReactNode,

  onConfirm(state: State | null): void,
}

export const ConfirmationsState = new class ConfirmationsState {
  public state: object | null = null;
  public request: ConfirmationRequest | null = null;

  constructor() {
    makeAutoObservable(this);
  }

  requestConfirm<State extends object = {}>(request: ConfirmationRequest<State>) {
    this.request = request;
  }

  readonly confirm = () => {
    if (!this.request) {
      return;
    }

    this.request.onConfirm(this.state);

    this.request = null;
    this.state = null;
  };

  readonly setState = <State extends object = {}>(state: State) => {
    this.state = state;
  };

  readonly close = () => {
    this.state = null;
    this.request = null;
  };
}();
