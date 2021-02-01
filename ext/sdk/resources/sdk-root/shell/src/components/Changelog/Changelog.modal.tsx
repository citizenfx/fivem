import * as React from 'react';
import { Modal } from 'components/Modal/Modal';
import { StateContext } from 'contexts/StateContext';
import { Changelog } from './Changelog';
import { Button } from 'components/controls/Button/Button';
import { ScrollContainer } from 'components/ScrollContainer/ScrollContainer';
import s from './Changelog.module.scss';

export const ChangelogModal = React.memo(function ChangelogModal() {
  const { closeChangelog } = React.useContext(StateContext);

  return (
    <Modal fullWidth onClose={closeChangelog}>
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
            onClick={closeChangelog}
          />
        </div>
      </div>
    </Modal>
  );
});
