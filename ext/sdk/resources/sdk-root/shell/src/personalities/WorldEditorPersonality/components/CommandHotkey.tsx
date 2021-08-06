import { Keystroke } from 'components/display/Keystroke/Keystroke';
import { observer } from 'mobx-react-lite';
import React from 'react';
import { WECommandType } from '../constants/commands';
import { WEHotkeysState } from '../store/WEHotkeysState';

export interface CommandHotkeyProps {
  command: WECommandType | void,
}

export const CommandHotkey = observer(function CommandHotkey(props: CommandHotkeyProps) {
  if (!props.command) {
    return null;
  }

  const keystroke = WEHotkeysState.getCommandHotkey(props.command);

  return (
    <Keystroke combination={keystroke} />
  );
});
