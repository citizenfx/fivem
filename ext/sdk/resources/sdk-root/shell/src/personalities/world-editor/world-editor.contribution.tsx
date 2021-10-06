import React from 'react';
import { IShellViewParticipant, ShellViewParticipants } from "fxdk/browser/shellExtensions";
import { ShellState } from "store/ShellState";
import { WorldEditorPersonality } from "./WorldEditorPersonality";

ShellViewParticipants.register(new class WorldEditorView implements IShellViewParticipant {
  readonly id = 'world-editor';

  isVisible() {
    return ShellState.isWorldEditorPersonality;
  }

  render() {
    return (
      <WorldEditorPersonality />
    );
  }
});
