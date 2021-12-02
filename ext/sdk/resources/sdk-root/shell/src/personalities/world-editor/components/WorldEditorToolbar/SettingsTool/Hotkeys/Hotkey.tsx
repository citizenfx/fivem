import React from 'react';
import { observer } from 'mobx-react-lite';
import { getCommandBinding } from 'personalities/world-editor/command-bindings';
import { WECommandType } from 'personalities/world-editor/constants/commands';
import { WEHotkeysState } from 'personalities/world-editor/store/WEHotkeysState';
import { div } from 'utils/styled';
import s from './Hotkeys.module.scss';
import { Keystroke } from 'fxdk/ui/Keystroke/Keystroke';
import { useOpenFlag } from 'utils/hooks';
import { BsCheck } from 'react-icons/bs';
import { closeIcon } from 'fxdk/ui/icons';
import { BiReset } from 'react-icons/bi';
import { getEventKeystroke } from 'utils/HotkeyController';

const Item = div<{ editing?: boolean }>(s, 'item');
const Title = div(s, 'title');
const Controls = div(s, 'controls');

const noop = () => {};

export interface HotkeyProps {
  command: WECommandType,

  editing: boolean,
  edit(command: WECommandType | null): void,
}

export const Hotkey = observer(function Hotkey(props: HotkeyProps) {
  const {
    command,
    editing,
    edit,
  } = props;

  const keystroke = WEHotkeysState.getCommandKeystroke(command);
  const commandBinding = getCommandBinding(command);

  const [intermediateKeystroke, setIntermediateKeystroke] = React.useState(keystroke);

  const handleCancel = React.useCallback(() => {
    setIntermediateKeystroke(WEHotkeysState.getCommandKeystroke(command));
    edit(null);
  }, [edit, command]);

  const handleSave = React.useCallback(() => {
    if (intermediateKeystroke) {
      WEHotkeysState.setCommandHotkey(command, intermediateKeystroke);
    }

    edit(null);
  }, [command, intermediateKeystroke, edit]);

  const handleResetToDefault = React.useCallback(() => {
    WEHotkeysState.resetCommandHotkey(command);
    setIntermediateKeystroke(WEHotkeysState.getCommandKeystroke(command));
    edit(null);
  }, [command, edit]);

  React.useEffect(() => {
    const handleKeyDown = (event: KeyboardEvent) => {
      if (!editing) {
        return;
      }

      event.preventDefault();
      event.stopPropagation();

      if (event.code === 'Escape') {
        return handleCancel();
      }
      if (event.code === 'Enter') {
        return handleSave();
      }

      const newKeystroke = getEventKeystroke(event);
      if (newKeystroke) {
        setIntermediateKeystroke(newKeystroke);
      }
    };

    document.addEventListener('keydown', handleKeyDown, true);

    return () => {
      document.removeEventListener('keydown', handleKeyDown, true);
    };
  }, [editing, command, handleCancel, handleSave]);

  return (
    <Item editing={editing} props={{ onClick: editing ? noop : () => edit(command) }}>
      <Title>
        {commandBinding.label || commandBinding.command}
      </Title>

      {intermediateKeystroke && <Keystroke combination={intermediateKeystroke} />}

      {editing && (
        <Controls>
          <button className={s.reset} onClick={handleResetToDefault}>
            <BiReset />
            Reset to default
          </button>
          <button className={s.cancel} onClick={handleCancel}>
            {closeIcon}
            Cancel
          </button>
          <button className={s.save} onClick={handleSave}>
            <BsCheck />
            Save
          </button>
        </Controls>
      )}
    </Item>
  );
});
