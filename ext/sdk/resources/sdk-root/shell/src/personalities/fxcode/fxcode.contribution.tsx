import React from 'react';
import { IShellViewParticipant, ShellViewParticipants } from "fxdk/browser/shellExtensions";
import { FXCodePersonality } from "./FXCodePersonality";
import { ShellState } from 'store/ShellState';
import { AppStates } from 'shared/api.types';
import { ProjectLoader } from 'fxdk/project/browser/state/projectLoader';

ShellViewParticipants.register(new class FXCodePersonalityView implements IShellViewParticipant {
  readonly id = 'fxcode';

  isVisible() {
    return ProjectLoader.hasProject && ShellState.appState === AppStates.ready;
  }

  render() {
    return (
      <FXCodePersonality />
    );
  }
});
