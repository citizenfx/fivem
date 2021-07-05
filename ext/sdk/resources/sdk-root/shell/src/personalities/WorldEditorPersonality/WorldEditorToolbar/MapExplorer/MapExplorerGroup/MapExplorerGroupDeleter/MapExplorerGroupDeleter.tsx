import React from 'react';
import { Button } from 'components/controls/Button/Button';
import { Checkbox } from 'components/controls/Checkbox/Checkbox';
import { Modal } from 'components/Modal/Modal';
import { deleteIcon } from 'constants/icons';
import { observer } from 'mobx-react-lite';
import { WorldEditorState } from 'personalities/WorldEditorPersonality/WorldEditorState';
import s from './MapExplorerGroupDeleter.module.scss';

export interface MapExplorerGroupDeleterProps {
  grp: string,
  onClose: () => void,
}

export const MapExplorerGroupDeleter = observer(function MapExplorerGroupDeleter(props: MapExplorerGroupDeleterProps) {
  const { grp, onClose } = props;

  const group = WorldEditorState.map.additionGroups[grp];
  const additions = Object.values(WorldEditorState.map.getGroupAdditions(grp));

  const [deleteAdditions, setDeleteAdditions] = React.useState(false);

  const handleDelete = React.useCallback(() => {
    onClose();

    WorldEditorState.map.deleteAdditionGroup(grp, deleteAdditions);
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
          label="Also delete them"
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
