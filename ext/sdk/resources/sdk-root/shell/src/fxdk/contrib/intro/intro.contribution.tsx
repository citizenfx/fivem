import React from 'react';
import { mapIcon } from 'constants/icons';
import { ShellLifecycle, ShellLifecyclePhase } from "fxdk/browser/shellLifecycle";
import { IShellViewParticipant, ShellViewParticipants } from "fxdk/browser/shellExtensions";
import { ProjectState } from "store/ProjectState";
import { ToolbarParticipants } from '../toolbar/toolbarExtensions';
import { IntroState } from "./IntroState";
import { Intro } from 'fxdk/ui/Intro/Intro';
import { introTourSteps } from './introSteps';

ShellViewParticipants.register(new class IntroViewParticipant implements IShellViewParticipant {
  readonly id = 'intro-tour';

  constructor() {
    ShellLifecycle.onPhase(ShellLifecyclePhase.Started, () => {
      if (IntroState.isFirstLaunch) {
        IntroState.open();
      }
    });
  }

  isVisible() {
    return ProjectState.hasProject && IntroState.isOpen;
  }

  render() {
    return (
      <Intro
        steps={introTourSteps}
        onFinish={IntroState.close}
      />
    );
  }
});

ToolbarParticipants.registerMenuItem({
  id: 'intro-tour',
  group: 'misc',
  item: {
    id: 'tour',
    text: 'Tour',
    icon: mapIcon,
    onClick: IntroState.open,
  },
});
