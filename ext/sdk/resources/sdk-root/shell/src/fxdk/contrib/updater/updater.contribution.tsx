import React from 'react';
import { AppStates } from 'shared/api.types';
import { ShellLifecycle, ShellLifecyclePhase } from 'fxdk/browser/shellLifecycle';
import { IShellViewParticipant, ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { ShellState } from 'store/ShellState';
import { Updater } from './Updater';
import { UpdaterState } from './UpdaterState';
import { UpdaterViewParticipants } from './updaterExtensions';

ShellViewParticipants.register(new class UpdaterView implements IShellViewParticipant {
  readonly id = 'updater';

  constructor() {
    ShellLifecycle.onPhase(ShellLifecyclePhase.Booting, () => {
      if (UpdaterViewParticipants.shouldShowUpdater()) {
        UpdaterState.open();
      }
    });
  }

  isVisible() {
    return ShellState.appState === AppStates.preparing || UpdaterState.isOpen;
  }

  render() {
    return (
      <Updater />
    );
  }
});
