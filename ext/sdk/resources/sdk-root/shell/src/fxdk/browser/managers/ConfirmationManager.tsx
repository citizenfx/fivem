import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { observer } from 'mobx-react-lite';
import { ConfirmationsState } from 'store/ConfirmationsState';

export const ConfirmationManager = observer(function ConfirmationManager() {
  if (ConfirmationsState.request === null) {
    return null;
  }

  return (
    <Modal onClose={ConfirmationsState.close}>
      <div className="modal-header">
        {ConfirmationsState.request.title}
      </div>

      {ConfirmationsState.request.children?.(ConfirmationsState.state, ConfirmationsState.setState) || null}

      <div className="modal-actions">
        <Button
          autofocus
          theme="primary"
          icon={ConfirmationsState.request.buttonIcon}
          text={ConfirmationsState.request.buttonText}
          onClick={ConfirmationsState.confirm}
        />

        <Button
          text="Cancel"
          onClick={ConfirmationsState.close}
        />
      </div>
    </Modal>
  );
});
