import React from 'react';
import classnames from 'classnames';
import s from './Hotkeys.module.scss';
import { observer } from 'mobx-react-lite';
import { Modal } from 'components/Modal/Modal';
import { div } from 'utils/styled';
import { WEState } from 'personalities/WorldEditorPersonality/store/WEState';
import { getAllCommandBindings, getAllCommands } from 'personalities/WorldEditorPersonality/command-bindings';
import { Hotkey } from './Hotkey';
import { WECommandType } from 'personalities/WorldEditorPersonality/constants/commands';

const Root = div(s, 'root');
const Wrapper = div<{ editing?: boolean }>(s, 'wrapper');

export interface HotkeysProps {
  onClose(): void,
}

export const Hotkeys = observer(function Hotkeys(props: HotkeysProps) {
  const { onClose } = props;

  const [editingCommand, setEditingCommand] = React.useState<WECommandType | null>(null);

  React.useEffect(() => {
    WEState.enterHotkeyConfigurationMode();

    return WEState.exitHotkeyConfigurationMode;
  }, []);

  const nodes = getAllCommandBindings()
    .filter(({ configurable }) => configurable)
    .map((binding) => (
      <Hotkey
        key={binding.command}
        command={binding.command}

        editing={editingCommand === binding.command}
        edit={setEditingCommand}
      />
    ));

  return (
    <Modal fullWidth fullHeight onClose={onClose}>
      <Root>
        <div className="modal-header">
          World Editor Hotkeys
        </div>

        <Wrapper editing={editingCommand !== null}>
          {nodes}
        </Wrapper>
      </Root>
    </Modal>
  );
});
