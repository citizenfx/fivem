import { Api } from 'fxdk/browser/Api';
import { makeAutoObservable } from 'mobx';
import { stateApi } from 'shared/api.events';
import { AppStates } from 'shared/api.types';

export enum ShellPersonality {
  MAIN,
  WORLD_EDITOR,
}

export const ShellState = new class ShellState {
  constructor() {
    makeAutoObservable(this);

    Api.on(stateApi.state, this.setAppState);
  }

  public personality = ShellPersonality.MAIN;
  setPersonality(personality: ShellPersonality) {
    this.personality = personality;
  }

  get isMainPersonality(): boolean {
    return this.personality === ShellPersonality.MAIN;
  }
  get isWorldEditorPersonality(): boolean {
    return this.personality === ShellPersonality.WORLD_EDITOR;
  }

  public appState = AppStates.booting;
  private setAppState = (state: AppStates) => this.appState = state;

  get isReady(): boolean {
    return this.appState === AppStates.ready;
  }

  private coverIframes = 0;

  get isIframeCovered(): boolean {
    return this.coverIframes > 0;
  }

  enableIframeCover() {
    this.coverIframes++;
  }
  disableIframeCover() {
    this.coverIframes--;
  }
}();
