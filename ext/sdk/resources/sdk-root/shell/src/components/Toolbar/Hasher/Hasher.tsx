import { Button } from 'components/controls/Button/Button';
import { Input } from 'components/controls/Input/Input';
import { Modal } from 'components/Modal/Modal';
import React from 'react';
import { joaat } from 'utils/joaat';

export const Hasher = React.memo(function Hasher({ close }: { close(): void }) {
  const [input, setInput] = React.useState('');

  return (
    <Modal onClose={close}>
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
          onClick={close}
        />
      </div>
    </Modal>
  );
});
