import * as React from 'react';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { Changelog } from './Changelog';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { ScrollContainer } from 'fxdk/ui/ScrollContainer/ScrollContainer';
import { observer } from 'mobx-react-lite';
import { ChangelogState } from './ChangelogState';
import s from './Changelog.module.scss';

export const ChangelogModal = observer(function ChangelogModal() {
  return (
    <Modal fullWidth onClose={ChangelogState.close}>
      <div className={s.modal}>
        <div className="modal-header">
          Changelog
        </div>

        <ScrollContainer className={s.changelog}>
          <Changelog />
        </ScrollContainer>

        <div className="modal-actions">
          <Button
            text="Close"
            onClick={ChangelogState.close}
          />
        </div>
      </div>
    </Modal>
  );
});
