export enum WECommandScope {
  EDITOR,
  PLAYTEST,
  CONFIGURATOR,
  INTRO,
}

export const WECommand = Object.freeze({
  TOOL_PATCHES_TOGGLE: 'tool.patches.toggle',
  TOOL_ADDITIONS_TOGGLE: 'tool.additions.toggle',
  TOOL_ADD_OBJECT_TOGGLE: 'tool.addObject.toggle',
  TOOL_ENVIRONMENT_TOGGLE: 'tool.environment.toggle',
  TOOL_SETTINGS_TOGGLE: 'tool.settings.toggle',

  CONTROL_COORD_SYSTEM_TOGGLE: 'control.coordSystem.toggle',
  CONTROL_MODE_TRANSLATE_TOGGLE: 'control.mode.translate.toggle',
  CONTROL_MODE_ROTATE_TOGGLE: 'control.mode.rotate.toggle',
  CONTROL_MODE_SCALE_TOGGLE: 'control.mode.scale.toggle',

  CONTROL_ESCAPE: 'control.escape',

  ACTION_DUPLICATE_SELECTION: 'action.duplicateSelection',
  ACTION_DELETE_SELECTION: 'action.deleteSelection',
  ACTION_SET_ADDITION_ON_GROUND: 'action.setAdditionOnGround',
  ACTION_ENTER_PLAYTEST_MODE: 'action.enterPlaytestMode',
  ACTION_FOCUS_SELECTION_IN_VIEW: 'action.focusSelectionInView',
});

export type WECommandType = (typeof WECommand)[keyof typeof WECommand];
