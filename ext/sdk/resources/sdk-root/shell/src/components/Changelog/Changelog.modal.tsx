import * as React from 'react';
import { Modal } from 'components/Modal/Modal';
import { Changelog } from './Changelog';
import { Button } from 'components/controls/Button/Button';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import { observer } from 'mobx-react-lite';
import { ShellState } from 'store/ShellState';
import s from './Changelog.module.scss';

export const ChangelogModal = observer(function ChangelogModal() {
  return (
    <Modal fullWidth onClose={ShellState.closeChangelog}>
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
            onClick={ShellState.closeChangelog}
          />
        </div>
      </div>
    </Modal>
  );
});
