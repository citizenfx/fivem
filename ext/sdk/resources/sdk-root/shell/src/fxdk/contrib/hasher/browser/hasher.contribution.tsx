import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Input } from 'fxdk/ui/controls/Input/Input';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { BsHash } from 'react-icons/bs';
import { ShellViewParticipants } from 'fxdk/browser/shellExtensions';
import { OpenFlag } from 'store/generic/OpenFlag';
import { joaat } from 'utils/joaat';
import { ToolbarParticipants } from '../../toolbar/browser/toolbarExtensions';

const HasherState = new OpenFlag(false);

const Hasher = React.memo(function Hasher() {
  const [input, setInput] = React.useState('');

  return (
    <Modal onClose={HasherState.close}>
      <div className="modal-header">
        JOAAT32 HASHER
      </div>

      <div className="modal-block">
        <Input
          autofocus
          value={input}
          onChange={setInput}
        />
      </div>

      <div className="modal-block">
        Hash:&nbsp;
        <code>{joaat(input).toString(16).toUpperCase()}</code>
      </div>

      <div className="modal-actions">
        <Button
          text="Close"
          onClick={HasherState.close}
        />
      </div>
    </Modal>
  );
});

ShellViewParticipants.register({
  id: 'hasher',
  isVisible: () => HasherState.isOpen,
  render: () => <Hasher />,
});

ToolbarParticipants.registerMenuItem({
  id: 'hasher',
  group: 'misc',
  item: {
    id: 'hasher',
    text: 'Hasher',
    icon: <BsHash />,
    onClick: HasherState.open,
  },
});
