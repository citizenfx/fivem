import React from 'react';
import { IShellViewParticipant, ShellViewParticipants } from "fxdk/browser/shellExtensions";
import { ProjectState } from 'store/ProjectState';
import { FXCodePersonality } from "./FXCodePersonality";
import { ShellState } from 'store/ShellState';
import { AppStates } from 'shared/api.types';

ShellViewParticipants.register(new class FXCodePersonalityView implements IShellViewParticipant {
  readonly id = 'fxcode';

  isVisible() {
    return ProjectState.hasProject && ShellState.appState === AppStates.ready;
  }

  render() {
    return (
      <FXCodePersonality />
    );
  }
});
