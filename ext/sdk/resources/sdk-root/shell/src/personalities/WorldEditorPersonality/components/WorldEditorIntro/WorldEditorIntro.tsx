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
          With FxDK's world editor you can modify the world to your desire! Place new objects, move or delete existing ones, we got you covered.
        </p>

        <p>
          The world editor is currently in alpha and many more features are coming. This includes placing more entities such as vehicles and checkpoints,
          but also allow you to create scripted content using a graphical editor. Read more about that <a href="#">here</a>! Stay tuned.
        </p>

        {
          WEState.introFirstTime
            ? (
              <p>
                Since this is your first time here, we'd like to guide you through the basics of using the editor.
                Throughout the tour you will learn about the controls, how to place and modify objects, create map patches, and how to use all the other tools.
              </p>
            )
            : (
              <p>
                In this tour we'd like to guide you through the basics of using the editor.
                You will learn about the controls, how to place and modify objects, create map patches, and how to use all the other tools.
              </p>
            )
        }
      </>
    ),
    closeButtonText: 'Skip',
    backButtonDisabled: true,
    nextButtonText: 'Start the tour',
    onEnter() {
      WEState.enterEditorMode();

      if (WEState.selection.type !== WESelectionType.NONE) {
        WEState.clearEditorSelection();
      }

      WEToolbarState.closeAllTools();
    }
  },
  // Would be great to include this little step to denote the use of different toolbars
  // This eases the progress of the tour
  // {
  //   content: () => (
  //     <>
  //       <h3>
  //         World modification toolbar
  //       </h3>

  //       <p>
  //         In the top left toolbar you will find the tools to place new elements in your world, and find the modifications you have made so far.
  //       </p>
  //     </>
  //   ),
  //   // Does not work, not sure if because of css or wrong data attr
  //   focusElement: 'toolbar-top-left',
  // },
  {
    content: () => (
      <>
        <h3>
          Map patches panel
          <CommandHotkey command={getToolCommand(WETool.Patches)} />
        </h3>

        <p>
          A <strong>map patch</strong> is a modification to existing map objects such as lamp poles, traffic lights, trash cans, etc.
        </p>

        <p>
          You can move, rotate, scale or delete existing map objects. To select an object, simply click on them.
          Any modification you will make, will appear here.
        </p>

        <p className="small">
          Note: not all objects on the map can be modified <em>yet</em>.<br/>
          Just click on something - if it highlights, you can patch it!
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
          <strong>Map additions</strong> are objects placed by <em>you</em>.
          Any object that you add to the map will show up in this panel. You can even make groups to organize your work!
        </p>
      </>
    ),
    focusElement: 'map-additions',
    onEnter() { WEToolbarState.openTool(WETool.Additions) },
    onExit() { WEToolbarState.closeTool(WETool.Additions) },
  },
  {
    content: () => (
      <>
        <h3>
          Add object panel
          <CommandHotkey command={getToolCommand(WETool.AddObject)} />
        </h3>

        <p>
          This is the object browser. Want to place a new object into your world? Find and add them from here.
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
        <h3>Properties panel</h3>

        <p>
          After placing or selecting an object, you can fine-tune the position/rotation/scale properties of the object here.
        </p>

        <div>
          You can modify each value with different controls:
          <ul>
            <li>Keyboard: use <Keystroke className={s.inline} combination="↑" /> <Keystroke className={s.inline} combination="↓" /> keys to change the value. The <Keystroke className={s.inline} combination="Shift" /> and <Keystroke className={s.inline} combination="Alt" /> modifier keys allow for a faster or slower change rate</li>
            <li>Mouse wheel: when hovering over the input field you can use your mouse wheel to change the value</li>
            <li>Click and drag: dragging the input field will also change the value</li>
          </ul>
        </div>
      </>
    ),
    focusElement: 'properties-panel',
    onEnter: WEToolbarState.showFakeProperties,
    onExit: WEToolbarState.hideFakeProperties,
  },
  // Can add a transparent image of a gizmo next to the text?
  // Perhaps even an animation that cycles through the 3 different modes?
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
          The <strong>Gizmo</strong> is the 3D tool which allows you to move, rotate or scale your selection. Use these buttons to change the gizmo mode; from move to rotate to scale.
        </p>
      </>
    ),
    focusElement: 'mode-selector',
  },
  {
    content: () => (
      <>
        <h3>
          Coordinate space
          <Keystroke combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_COORD_SYSTEM_TOGGLE)} />
        </h3>

        <p>
          When you rotate an object, you rotate its axis. With this tool you can change the behavior of moving or scaling your selection:
        </p>

        <ul>
          <li>Local coordinate space: Move along the axis of your selection</li>
          <li>Global coordinate space: Move your selection along the world axis</li>
        </ul>

        <p>
          Try it out yourself to get a feel of how it works!
        </p>
      </>
    ),
    focusElement: 'coord-system',
  },
  {
    content: () => (
      <>
        <h3>
          Play mode

          <CommandHotkey command={WECommand.ACTION_ENTER_PLAYTEST_MODE} />
        </h3>

        <p>
          Quickly jump into your world to test your work.
        </p>

        <p>
          You can also choose to spawn in a vehicle, which you can enable from the Settings panel.
        </p>

        <p>
          <em>Press <Keystroke className={s.inline} combination="ESC" /> to go back to the editor.</em>
        </p>
      </>
    ),
    focusElement: 'play-test',
  },
  {
    content: () => (
      <>
        <h3>
          Environment panel

          <CommandHotkey command={getToolCommand(WETool.Environment)} />
        </h3>

        <p>
          You can control the time and weather here.
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
        <h3>
          Settings panel

          <CommandHotkey command={getToolCommand(WETool.Settings)} />
        </h3>

        <p>
          Knobs, ticks and sliders. You can take this tour again anytime from here.
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
          Camera controls
        </h3>

        <p>
          Click and hold your <strong>Right Mouse Button</strong> to control the camera rotation.
        </p>

        <p>
          Use your <strong>Left Mouse Button</strong> to select objects.
        </p>

        <div>
          Move the camera around using the&nbsp;
          <div style={{ display: 'inline-flex', gap: '2px' }}>
            <Keystroke combination={KeyboardLayout.get('KeyW')}/>
            <Keystroke combination={KeyboardLayout.get('KeyA')}/>
            <Keystroke combination={KeyboardLayout.get('KeyS')}/>
            <Keystroke combination={KeyboardLayout.get('KeyD')}/>
          </div>
          &nbsp;keys.<br/>
          <span>
            You can use <Keystroke className={s.inline} combination="Shift" /> to increase the camera speed or <Keystroke className={s.inline} combination="Alt" /> to move slower.
          </span>
        </div>
      </>
    ),
  },
  {
    content: () => (
      <>
        <h3>
          Keyboard shortcuts
        </h3>

        <p>
          The world editor is convenient to use from your keyboard. Here's a quick list of the most useful shortcuts.
        </p>

        <p>Objects:</p>
        <ul>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.TOOL_ADD_OBJECT_TOGGLE)} /> - Find and add a new object</li>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.ACTION_SET_ADDITION_ON_GROUND)} /> - Place your selection flush on the ground</li>
        </ul>

        <p>Selection:</p>
        <ul>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_COORD_SYSTEM_TOGGLE)} /> - Switch between local and global coordinate space</li>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_TRANSLATE_TOGGLE)} /> - Move selection</li>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_ROTATE_TOGGLE)} /> - Rotate selection</li>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.CONTROL_MODE_SCALE_TOGGLE)} /> - Scale selection</li>
        </ul>

        <p>Testing:</p>
        <ul>
          <li><Keystroke className={s.inline} combination={WEHotkeysState.getCommandHotkey(WECommand.ACTION_ENTER_PLAYTEST_MODE)} /> - Jump into play mode</li>
          <li><Keystroke className={s.inline} combination="ESC" /> - Exit play mode</li>
        </ul>
      </>
    ),
  },
  {
    content: () => (
      <>
        <h3>That's all!</h3>

        <p>
          Get creative! It's your turn to create your own unique world.
        </p>

        <p>
          Make sure to read all about the <a href="#">upcoming features</a> in the FxDK world editor. There is much more to come.
        </p>
      </>
    ),
    // FIX: Back button is placed on the close button position here
    nextButtonText: 'Get started!',
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
