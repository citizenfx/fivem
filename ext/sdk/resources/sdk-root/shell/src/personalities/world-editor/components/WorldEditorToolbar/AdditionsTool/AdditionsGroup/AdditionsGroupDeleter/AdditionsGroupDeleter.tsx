import React from 'react';
import { Button } from 'fxdk/ui/controls/Button/Button';
import { Checkbox } from 'fxdk/ui/controls/Checkbox/Checkbox';
import { Modal } from 'fxdk/ui/Modal/Modal';
import { deleteIcon } from 'fxdk/ui/icons';
import { observer } from 'mobx-react-lite';
import { WEState } from 'personalities/world-editor/store/WEState';
import s from './AdditionsGroupDeleter.module.scss';

export interface AdditionsGroupDeleterProps {
  grp: string,
  onClose: () => void,
}

export const AdditionsGroupDeleter = observer(function AdditionsGroupDeleter(props: AdditionsGroupDeleterProps) {
  const { grp, onClose } = props;

  const group = WEState.map!.additionGroups[grp];
  const additions = Object.values(WEState.map!.getGroupAdditions(grp));

  const [deleteAdditions, setDeleteAdditions] = React.useState(false);

  const handleDelete = React.useCallback(() => {
    onClose();

    WEState.map!.deleteAdditionGroup(grp, deleteAdditions);
  }, [grp, onClose, deleteAdditions]);

  return (
    <Modal onClose={onClose}>
      <div className="modal-header">
        Delete "{group.label}" group?
      </div>

      <div className="modal-label">
        Group has following additions:
      </div>
      <div className="modal-block">
        <ul className={s.additions}>
          {additions.slice(0, 5).map(({ label }, i) => (
            <li key={`${i}-${label}`}>
              {label}
            </li>
          ))}
          {
            additions.length > 5
              ? (
                <li>And {additions.length - 5} more</li>
              )
              : null
          }
        </ul>
      </div>

      <div className="modal-block">
        <Checkbox
          value={deleteAdditions}
          onChange={setDeleteAdditions}
          label="Also delete additions"
        />
      </div>

      <div className="modal-actions">
        <Button
          text="Delete"
          icon={deleteIcon}
          onClick={handleDelete}
        />

        <Button
          autofocus
          text="Cancel"
          theme="primary"
          onClick={onClose}
        />
      </div>
    </Modal>
  );
});
