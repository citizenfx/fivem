import { Button } from 'components/controls/Button/Button';
import { Modal } from 'components/Modal/Modal';
import { observer } from 'mobx-react-lite';
import React from 'react';
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
