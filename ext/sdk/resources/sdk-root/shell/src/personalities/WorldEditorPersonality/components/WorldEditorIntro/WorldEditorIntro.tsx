import React from 'react';
import { Keystroke } from 'components/display/Keystroke/Keystroke';
import { Intro, IntroStep } from 'components/Intro/Intro';
import { WECommand } from 'personalities/WorldEditorPersonality/constants/commands';
import { WEHotkeysState } from 'personalities/WorldEditorPersonality/store/WEHotkeysState';
import { getToolCommand, WETool, WEToolbarState } from 'personalities/WorldEditorPersonality/store/WEToolbarState';
import { KeyboardLayout } from 'store/KeyboardLayout';
import { CommandHotkey } from '../CommandHotkey';
import s from './WorldEditorIntro.module.scss';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { WESelectionType } from 'backend/world-editor/world-editor-types';

const steps: IntroStep[] = [
  {
    content: () => (
      <>
        <h1>
          <span>
            Welcome to the&nbsp;<strong>World Editor</strong><sup><em>α</em></sup>
          </span>
        </h1>

        <p>
          Ever wanted to easily add that random cantainer on map?
          <br/>
          May be install toll booth?
          <br/>
          Put some awesome jump ramp on a beach?
        </p>

        <p>
          We got you covered.
        </p>
      </>
    ),
    nextButtonText: 'Okay',
    onEnter() {
      WEState.enterEditorMode();

      if (WEState.selection.type !== WESelectionType.NONE) {
        WEState.clearEditorSelection();
      }

      WEToolbarState.closeAllTools();
    }
  },
  {
    content: () => (
      <>
        <p>
          If you don't mind, we'll take a little tour on an interface of the&nbsp;World&nbsp;Editor.
        </p>

        <p>
          <em>Or, you can click the <strong>Close</strong> button and dive right in.</em>
        </p>
      </>
    ),
    nextButtonText: 'Yes, to the tour',
  },
  {
    content: () => (
      <>
        <h3>
          Map patches panel
          <CommandHotkey command={getToolCommand(WETool.Patches)} />
        </h3>

        <p>
          <strong>Map patch</strong> - is a modification of existing map object: lamp poles, traffic lights, trash cans, etc.
        </p>

        <p>
          Not all objects on map can be modified, <em>yet</em>.
          <br/>
          Just click on something and if it highlights - you can patch it!
        </p>
      </>
    ),
    focusElement: 'map-patches',
    onEnter() { WEToolbarState.openTool(WETool.Patches) },
    onExit() { WEToolbarState.closeTool(WETool.Patches) },
  },
  {
    content: () => (
      <>
        <h3>
          Map additions panel
          <CommandHotkey command={getToolCommand(WETool.Additions)} />
        </h3>

        <p>
          <strong>Map addition</strong> - is an object added by <em>you</em>.
        </p>
      </>
    ),
    nextButtonText: 'Thanks, Captain Obvious!',
    focusElement: 'map-additions',
    onEnter() { WEToolbarState.openTool(WETool.Additions) },
    onExit() { WEToolbarState.closeTool(WETool.Additions) },
  },
  {
    content: () => (
      <>
        <h3>Properties panel</h3>

        <p>
          Fine-tune position/rotation/scale of selected object.
        </p>

        <div>
          Each input value can be altered by:
          <ul>
            <li>
              Keyboard: use <Keystroke className={s.inline} combination="↑" /> <Keystroke className={s.inline} combination="↓" /> keys combined with <Keystroke className={s.inline} combination="Shift" /> or <Keystroke className={s.inline} combination="Alt" /> modifiers for faster or slower change rate</li>
            <li>Mouse wheel: hover the input field and use your mouse wheel to change value</li>
            <li>Mouse drag: dragging input field will also change value</li>
          </ul>
        </div>

        <div>
          <div style={{ display: 'inline-flex' }}><CommandHotkey command={WECommand.ACTION_SET_ADDITION_ON_GROUND} /></div>
          &nbsp;<em>Set object on ground</em> command also available for additions.
        </div>
      </>
    ),
    focusElement: 'properties-panel',
    onEnter: WEToolbarState.showFakeProperties,
    onExit: WEToolbarState.hideFakeProperties,
  },
  {
    content: () => (
      <>
        <h3>
          Add object panel
          <CommandHotkey command={getToolCommand(WETool.AddObject)} />
        </h3>

        <p>
          This is where you pick what object will become your next map addition.
        </p>
      </>
    ),
    focusElement: 'add-object',
    onEnter() { WEToolbarState.openTool(WETool.AddObject) },
    onExit() { WEToolbarState.closeTool(WETool.AddObject) },
  },
  {
    content: () => (
      <>
        <h3>
          Gizmo mode selector

          <div>
            <Keystroke combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_TRANSLATE_TOGGLE)} />
            <Keystroke combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_ROTATE_TOGGLE)} />
            <Keystroke combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_SCALE_TOGGLE)} />
          </div>
        </h3>

        <p>
          <strong>Gizmo</strong> - is a 3D control, shows up when you select something, either addition or patch, allows you to change selection position, rotation and scale.
        </p>
      </>
    ),
    focusElement: 'mode-selector',
  },
  {
    content: () => (
      <>
        <h3>
          Global or local coordinates
          <Keystroke combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_COORD_SYSTEM_TOGGLE)} />
        </h3>

        <p>
          If you're unfamiliar with this concept - best if you try it out yourself.
        </p>
      </>
    ),
    focusElement: 'coord-system',
  },
  {
    content: () => (
      <>
        <h3>
          Camera controls
        </h3>

        <p>
          <strong>Right Mouse Button</strong> to control camera rotation.
        </p>

        <p>
          <strong>Left Mouse Button</strong> to select objects.
        </p>

        <div>
          <div style={{ display: 'inline-flex', gap: '2px' }}>
            <Keystroke combination={KeyboardLayout.get('KeyW')}/>
            <Keystroke combination={KeyboardLayout.get('KeyA')}/>
            <Keystroke combination={KeyboardLayout.get('KeyS')}/>
            <Keystroke combination={KeyboardLayout.get('KeyD')}/>
          </div>
          <span>
           &nbsp;to move camera, combine with <Keystroke className={s.inline} combination="Shift" /> or <Keystroke className={s.inline} combination="Alt" /> for faster or slower movement.
          </span>
        </div>
      </>
    ),
  },
  {
    content: () => (
      <>
        <h3>
          Play test mode

          <CommandHotkey command={WECommand.ACTION_ENTER_PLAYTEST_MODE} />
        </h3>

        <p>
          Test your creation right away.
        </p>

        <p>
          You can also choose to spawn in a vehicle, enable it in the Settings panel.
        </p>

        <p>
          <em>Press ESC to exit play test mode.</em>
        </p>
      </>
    ),
    focusElement: 'play-test',
  },
  {
    content: () => (
      <>
        <h3>
          Settings panel

          <CommandHotkey command={getToolCommand(WETool.Settings)} />
        </h3>

        <p>
          Knobs, ticks and sliders, including the <em>Field of Flew Stretcher©®™</em>.
        </p>
      </>
    ),
    focusElement: 'settings-panel',
    onEnter() { WEToolbarState.openTool(WETool.Settings) },
    onExit() { WEToolbarState.closeTool(WETool.Settings) },
  },
  {
    content: () => (
      <>
        <h3>
          Environment panel

          <CommandHotkey command={getToolCommand(WETool.Environment)} />
        </h3>

        <p>
          Allows you to control time and weather conditions.
        </p>
      </>
    ),
    focusElement: 'environment-panel',
    onEnter() { WEToolbarState.openTool(WETool.Environment) },
    onExit() { WEToolbarState.closeTool(WETool.Environment) },
  },
  {
    content: () => (
      <>
        <h3>That's all, folks!</h3>

        <p>
          But don't get fooled, it's only the beginning.
        </p>
      </>
    ),
    nextButtonText: 'Finally, let me try this already',
  },
];

export function WorldEditorIntro() {
  return (
    <Intro
      steps={steps}
      onFinish={WEState.closeIntro}
    />
  );
}
