import { hasNewChangelogEntries } from 'components/Changelog/Changelog.utils';
import { makeAutoObservable } from 'mobx';
import { stateApi } from 'shared/api.events';
import { AppStates } from 'shared/api.types';
import { onApiMessage, sendApiMessage } from 'utils/api';

export const ShellState = new class ShellState {
  constructor() {
    makeAutoObservable(this);

    onApiMessage(stateApi.state, this.setAppState);
  }

  public ack() {
    sendApiMessage(stateApi.ackState);
  }

  public appState = AppStates.booting;
  private setAppState = (state: AppStates) => this.appState = state;

  get isReady(): boolean {
    return this.appState === AppStates.ready;
  }

  public updaterOpen = hasNewChangelogEntries();
  openUpdater = () => this.updaterOpen = true;
  closeUpdater = () => this.updaterOpen = false;

  public changelogOpen = false;
  openChangelog = () => this.changelogOpen = true;
  closeChangelog = () => this.changelogOpen = false;
};
